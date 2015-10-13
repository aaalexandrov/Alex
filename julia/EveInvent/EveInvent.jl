module EveInvent

import Requests
import HttpServer
import URIParser
import JSON
import LightXML
import Dates
import DataFrames

include("crest.jl")
include("crestauth.jl")
include("apiv2.jl")
include("evec.jl")
include("dataimp.jl")

global config
global services, regions, itemCategories, itemTypes, itemNames
global itemPrices = nothing
global marketRegion, marketGroups, marketPrices, industrySystems, industryFacilities, assemblyLines
global shipTypes, blueprintTypes, blueprints, inventableProducts, productionProducts, shipInventable
global decryptors, skills

global charSkills, charBlueprints, charBPOTypes

global crestAuth = nothing

const priceTimeout = 6.0

id_from_url(url::AbstractString) = int(rsplit(url, '/', 2, false)[end])

function access_path(o, p::Array)
	for i in p
		if !(isa(o, Associative) && haskey(o, i) || isa(o, AbstractArray) && 1 <= i <= length(o))
			return nothing
		end
		o = o[i]
	end
	return o
end

function server_version(serv)
	strVer = match(r"[0-9|.?]+", serv["serverVersion"])
	map(int, split(strVer.match, '.'))
end

function get_services() 
	oldServ = read_from_cache(urlCrest, inf(Float64))
	newServ = get_crest(urlCrest, crestAuth, 0.0)
	@assert newServ != nothing
	if oldServ == nothing || server_version(oldServ)[1:2] != server_version(newServ)[1:2]
		info("Server version differs from cached data, clearing cache")
		clear_cache(".json")
		write_to_cache(urlCrest, newServ)
	end
	return newServ
end

get_service(path::Array) = access_path(services, path)["href"]

make_row_dict(colNames, vals) = Dict(filter(nv->!DataFrames.isna(nv[2]), zip(colNames, vals)))

function make_array(df::DataFrames.DataFrame)
	colNames = map(string, names(df))
	return [make_row_dict(colNames, vals) for vals in zip(DataFrames.columns(df)...)]
end

function make_index(key::Symbol, df::DataFrames.DataFrame)
	colNames = map(string, names(df))
	keyIndex = indexin([key], names(df))[1]
	keyType = eltype(df[key])
	return [vals[keyIndex]::keyType => make_row_dict(colNames, vals) for vals in zip(DataFrames.columns(df)...)]
end

function make_index(kvFunc::Function, collection, target)
	for e in collection
		k,v = kvFunc(e)
		target[k] = v
	end
	return target
end

function get_regions()
	jsonRegions = get_crest(get_service(["regions"]), crestAuth)
	make_index(jsonRegions, Dict{Int, Any}()) do r
		id_from_url(r["href"]), r
	end
end

get_region(id::Int) = get_crest(regions[id]["href"], crestAuth)

get_item_types() = make_index(t->(id_from_url(t["href"]), t), get_crest(get_service(["itemTypes"]), crestAuth), Dict{Int, Any}())
get_item_names() = [t["name"]=>id for (id, t) in itemTypes]

get_item_orders(typeID::Int, sellOrders::Bool) = get_crest(marketRegion[sellOrders? "marketSellOrders" : "marketBuyOrders"]["href"] * "?type=" * itemTypes[typeID]["href"], crestAuth, priceTimeout)

function resolve_item_prices(types)
	global itemPrices
	if itemPrices == nothing
		itemPrices = Dict{Int, Any}()
	end
	missing = filter(t->!haskey(itemPrices, t), types)
	for typeID in missing
		sell = get_item_orders(typeID, true)
		sellPrice = reduce(min, inf(Float64), map(o->o["price"], sell))
		# buy = get_item_orders(typeID, false)
		# buyPrice = reduce(max, 0.0, map(o->o["price"], buy))
		itemPrices[typeID] = [#="buy"=>buyPrice,=# "sell"=>sellPrice]
	end
end

function resolve_item_prices_evec(types)
	global itemPrices
	if itemPrices == nothing
		itemPrices = json_read("cache/itemPrices.json")
		if itemPrices == nothing
			itemPrices = Dict{Int, Any}()
		end
	end
	timeNow = time()
	missing = filter(t->!haskey(itemPrices, t) || itemPrices[t]["timeStamp"]<=timeNow, types)
	if isempty(missing)
		return
	end
	timeStamp = timeNow + priceTimeout * 60 * 60
	res = get_evec("marketstat", query={"typeid"=>join(missing, ','), "regionlimit"=>config["marketRegionID"]}; timeoutHours=0.0)
	for typeID in missing
		buy = xml_find(res, "marketstat/id:$typeID/buy/max")[1, 1]
		sell = xml_find(res, "marketstat/id:$typeID/sell/min")[1, 1]
		itemPrices[typeID] = ["buy"=>buy, "sell"=>sell, "timeStamp"=>timeStamp]
	end
	json_write("cache/itemPrices.json", itemPrices)
end

get_item_categories() = get_crest(get_service(["itemCategories"]), crestAuth)
get_item_category(catName::AbstractString) = get_crest(filter(c->c["name"]==catName, itemCategories)[1]["href"], crestAuth)

function get_category_types(category)
	types = Dict{Int, Any}()
	if category == nothing
		return types
	end
	if haskey(category, "types")
		make_index(category["types"], types) do t
		    id_from_url(t["href"]), t["name"]
		end
	end
	if haskey(category, "groups")
		for group in category["groups"]
			subtypes = get_category_types(get_crest(group["href"], crestAuth))
			merge!(types, subtypes)
		end
	end
	return types
end

get_category_types(catName::AbstractString) = get_category_types(get_item_category(catName))

get_market_groups() = get_crest(get_service(["marketGroups"]), crestAuth)
get_market_group(group::AbstractString) = filter(g->g["name"]==group, marketGroups)[1]

function get_group_types(group::AbstractString)
	typeGroup = get_market_group(group)
	jTypes = get_crest(typeGroup["types"]["href"], crestAuth)
	make_index(jTypes, Dict{Int, AbstractString}()) do t
		t["type"]["id"], t["type"]["name"]
	end
end

function get_market_prices() 
	priceArray = get_crest(get_service(["marketPrices"]), crestAuth, priceTimeout)
	make_index(priceArray, Dict{Int, Any}()) do p
		p["type"]["id"], p
	end
end

function get_industry_facilities()
	facilities = get_crest(get_service(["industry", "facilities"]), crestAuth)
	make_index(facilities, Dict{Int, Any}()) do f
		f["facilityID"], f
	end
end

function get_industry_systems()
	systems = get_crest(get_service(["industry", "systems"]), crestAuth)
	make_index(systems, Dict{Int, Any}()) do s
		s["solarSystem"]["id"], s
	end
end

function get_or_import(fileName::AbstractString, importFunc::Function)
	data = json_read(fileName)
	if data == nothing
		info("Importing $fileName from static data...")
		importFunc(fileName)
		data = json_read(fileName)
	end
	return data
end

function get_decryptors() 
	decr = get_or_import("data/decryptors.json", import_decryptors)
	noDecr = ["name"=>"No Decryptor", "attributes"=>["inventionMEModifier"=>0.0, "inventionMaxRunModifier"=>0.0, "inventionTEModifier"=>0.0, "inventionPropabilityMultiplier"=>1.0]]
	push!(decr, noDecr)
	return decr
end

get_assembly_lines() = get_or_import("data/assemblylines.json", import_station_assembly_lines)
get_blueprints() = get_or_import("data/blueprints.json", import_blueprints)

get_authorization() = request_access(get_service(["authEndpoint"]), config["appInfo"])

function get_config()
	config = json_read("config.json")
	config["appInfo"] = json_read("appinfo.json")
	set_api_user_agent(config["appInfo"]["userAgent"])
	characters = get_characters(config)
	characterInd = indexin([config["charName"]], characters[:name])[1]
	config["charID"] = ["characterID"=>string(characters[characterInd, :characterID])]
	return config
end

get_characters(config) = xml_find(get_api("Account/Characters"; auth=config["api"]), "result/:characters/*/*:")
get_char_skills() = xml_find(get_api("Char/CharacterSheet"; params=config["charID"], auth=config["api"]), "result/:skills/*/*:")
get_char_assets() = xml_find(get_api("Char/AssetList"; params=config["charID"], auth=config["api"]), "result/:assets/*/*:")
get_char_blueprints() = xml_find(get_api("Char/Blueprints"; params=config["charID"], auth=config["api"]), "result/:blueprints/*/*:")
get_skills() = xml_find(get_api("Eve/SkillTree"), "result/:skillGroups/*/:skills/*/*:")

function process_market_groups()
	global shipTypes = get_category_types("Ship")
	global blueprintTypes = get_category_types("Blueprint")
end

get_products(blueprint, activity::AbstractString) = access_path(blueprint, ["activities", activity, "products"])

function get_inventable_blueprints(groupTypes::Dict{Int})
	groupPrints = Set{Int}()
	for (b,bv) in blueprints
		invProducts = get_products(bv, "invention")
		if invProducts == nothing
			continue
		end
		for prod in invProducts
			t2BlueprintID = prod["typeID"]
			t2Blueprint = get(blueprints, t2BlueprintID, nothing)
			t2Product = get_products(t2Blueprint, "manufacturing")[1]["typeID"]
			if haskey(groupTypes, t2Product) && haskey(blueprintTypes, b) # need to check if we're looking at a blueprint because of the t3 inventable research items
				push!(groupPrints, t2Product)
			end
		end
	end
	return groupPrints
end

function process_products(f::Function, blueprint, activity::AbstractString)
	prods = access_path(blueprint, ["activities", activity, "products"])
	if prods != nothing
		for p in prods
			f(p)
		end
	end
end

function process_blueprints()
	global inventableProducts = Dict{Int, Any}()
	global productionProducts = Dict{Int, Any}()
	for (bid, bp) in blueprints
		process_products(bp, "invention") do prod
			inventableProducts[prod["typeID"]] = bp
		end
		process_products(bp, "manufacturing") do prod
			productionProducts[prod["typeID"]] = bp
		end
	end
	global shipInventable = get_inventable_blueprints(shipTypes)
	
	global charBPOTypes = Dict{Int, Array}()
	for (itemID, item) in charBlueprints
		if item["runs"] >= 0
			continue
		end
		itemArr = get!(charBPOTypes, item["typeID"], Array(Dict, 0))
		push!(itemArr, item)
	end
end

system_id(systemName::AbstractString) = first(filter((id,s)->s["solarSystem"]["name"]==systemName, industrySystems))[1]

function process_industry()
	for line in assemblyLines
		system = industrySystems[line["solarSystemID"]]
		lines = get!(system, "assemblyLines", Array(Dict, 0))
		push!(lines, line)
	end
	config["productionSystemID"] = system_id(config["productionSystem"])
	config["inventionSystemID"] = system_id(config["inventionSystem"])
	config["marketRegionID"] = first(filter(f->f["solarSystem"]["id"]==config["productionSystemID"], values(industryFacilities)))["region"]["id"]
	global marketRegion = get_region(config["marketRegionID"])
	
end

function production_amount(runs::Float64, baseQuantity::Float64, ME::Int, facilityMultiplier::Float64 = 1.0)
	materialModifier = facilityMultiplier * (1 - ME / 100.0)
	return max(runs, ceil(round(runs * baseQuantity * materialModifier, 2)))
end

function production_time(runs::Float64, baseProductionTime::Float64, TE::Int, skills::Array{Int}, facilityMultiplier::Float64 = 1.0)
	timeModifier = facilityMultiplier * (1 - TE / 100.0)
	skillModifier = prod(map(s->(1 - s / 100.0), skills))
	return baseProductionTime * timeModifier * skillModifier * runs
end

adjusted_price(itemID::Int) = marketPrices[itemID]["adjustedPrice"]

function base_job_cost(productID::Int)
	if haskey(blueprints, productID)
		materials = blueprints[productID]["activities"]["manufacturing"]["materials"]
	else
		materials = productionProducts[productID]["activities"]["manufacturing"]["materials"]
	end
	cost = 0.0
	for m in materials
		cost += m["quantity"] * adjusted_price(m["typeID"])
	end
	return cost
end

function production_install_cost(productID::Int, systemCostIndex::Float64, facilityTaxRate::Float64)
	baseCost = base_job_cost(productID)
	return baseCost * systemCostIndex * (1.0 + facilityTaxRate)
end

function invention_copy_install_cost(runs::Float64, blueprintID::Int, systemCostIndex::Float64, facilityTaxRate::Float64)
	baseCost = base_job_cost(blueprintID)
	return baseCost * 0.02 * systemCostIndex * (1.0 + facilityTaxRate) * runs
end

function is_encryption(skillID::Int)
	skillName = skills[skillID]["typeName"]
	return !isempty(search(skillName, "Encryption Methods"))
end

function invention_chance(baseChance::Float64, skills::Array, decryptorModifier::Float64)
	skillModifier = 1.0
	for s in skills
		skillID = s["typeID"]
		if haskey(charSkills, skillID)
			level = charSkills[skillID]["level"]
		else
			level = 0
			info("Missing character skill " * itemTypes[skillID]["name"])
		end
		skillModifier += is_encryption(skillID)? level/40.0 : level/30.0
	end
	return baseChance * skillModifier * decryptorModifier
end

function get_cost_index(system, activityName::AbstractString)
	filter(c->c["activityName"]==activityName, system["systemCostIndices"])[1]["costIndex"]
end

function get_tax_rate(system, activityName::AbstractString)
	stationID = filter(l->l["activityName"]==activityName, system["assemblyLines"])[1]["stationID"]
	return industryFacilities[stationID]["tax"]
end

function blueprint_materials(runs::Float64, blueprintItem, systemID::Int)
	blueprint = blueprints[blueprintItem["typeID"]]
	ME = blueprintItem["materialEfficiency"]
	productID = blueprint["activities"]["manufacturing"]["products"][1]["typeID"]
	baseMaterials = blueprint["activities"]["manufacturing"]["materials"]
	materials = [m["typeID"]::Int => production_amount(runs, float64(m["quantity"]), ME) for m in baseMaterials]
	system = industrySystems[systemID]
	installCost = production_install_cost(productID, get_cost_index(system, "Manufacturing"), get_tax_rate(system, "Manufacturing"))
	return materials, installCost
end

function material_cost(materials)
	resolve_item_prices(keys(materials))
	cost = 0.0
	for (matID, matAmount) in materials
		cost += itemPrices[matID]["sell"] * matAmount
	end
	return cost
end

function invention_result(targetID::Int, decryptor, systemID::Int=config["inventionSystemID"])
	baseRuns = 1.0
	baseME = 2.0
	basePE = 4.0
	baseProbability = 1.0
	t2BP = productionProducts[targetID]
	t2BPID = t2BP["blueprintTypeID"]
	t1BP = inventableProducts[t2BPID]
	invention = t1BP["activities"]["invention"]
	product = filter(p->p["typeID"]==t2BPID, invention["products"])[1]
	baseProbability = invention_chance(baseProbability * product["probability"], invention["skills"], decryptor["attributes"]["inventionPropabilityMultiplier"])
	baseME += decryptor["attributes"]["inventionMEModifier"]
	basePE += decryptor["attributes"]["inventionTEModifier"]
	baseRuns += decryptor["attributes"]["inventionMaxRunModifier"]
	resultItem = Dict{AbstractString, Any}()
	resultItem["typeID"] = t2BPID
	resultItem["materialEfficiency"] = int(baseME)
	resultItem["timeEfficiency"] = int(basePE)
	resultItem["probability"] = baseProbability
	resultItem["runs"] = baseRuns
	resultItem["quantity"] = product["quantity"]
	materials = Dict{Int, Float64}()
	for m in invention["materials"]
		materials[m["typeID"]] = m["quantity"]
	end
	if haskey(decryptor, "id")
		materials[decryptor["id"]] = 1
	end
	system = industrySystems[systemID]
	copyCost = invention_copy_install_cost(1.0, t1BP["blueprintTypeID"], get_cost_index(system, "Copying"), get_tax_rate(system, "Copying"))
	installCost = invention_copy_install_cost(1.0, targetID, get_cost_index(system, "Invention"), get_tax_rate(system, "Invention"))
	materialCost = material_cost(materials)
	attemptsPerCopy = baseRuns * product["quantity"] / baseProbability
	return resultItem, (materialCost + installCost + copyCost) * attemptsPerCopy
end

function add_materials!(target, source, multiplier)
	for (matID, matAmount) in source
		target[matID] = get(target, matID, 0.0) + matAmount * multiplier
	end
	return target
end

function get_char_bpo(productID::Int, systemID::Int)
	if !haskey(productionProducts, productID)
		return nothing
	end
	bp = productionProducts[productID]
	bpID = bp["blueprintTypeID"]
	if !haskey(charBPOTypes, bpID)
		return nothing
	end
	localBPOs = filter(charBPOTypes[bpID]) do item
		access_path(industryFacilities, [item["locationID"], "solarSystem", "id"]) == systemID
	end
	if isempty(localBPOs)
		localBPOs = charBPOTypes[bpID]
	end
	return localBPOs[1]
end

function break_materials(runs::Float64, blueprintItem, systemID::Int=config["productionSystemID"])
	materials, installCost = blueprint_materials(runs, blueprintItem, systemID)
	intermediates = Dict{Int, Float64}()
	while true
		bpItem = nothing
		local matID, matAmount
		for (matID, matAmount) in materials
			bpItem = get_char_bpo(matID, systemID)
			if bpItem != nothing
				break
			end
		end
		if bpItem == nothing
			break
		end
		intermediates[matID] = get(intermediates, matID, 0.0) + matAmount
		mats, cost = blueprint_materials(matAmount, bpItem, systemID)
		delete!(materials, matID)
		add_materials!(materials, mats, 1.0)
		installCost += cost
	end
	return materials, intermediates, installCost
end

function optimal_invention(targetID::Int)
	resolve_item_prices([targetID])
	price = itemPrices[targetID]["sell"]
	optimal = (-inf(Float64), nothing)
	for decr in decryptors
		resultItem, inventCost = invention_result(targetID, decr)
		materials, intermediates, produceInstall = break_materials(1.0, resultItem)
		totalCost = inventCost + material_cost(materials) + produceInstall
		profit = price - totalCost
		if optimal[1] < profit
			optimal = (profit, decr)
		end
	end
	return optimal
end

function optimal_invention(checkBPOs::Bool = true)
	result = {}
	for ship in shipInventable
		t2BP = productionProducts[ship]
		if checkBPOs
			t1BP = inventableProducts[t2BP["blueprintTypeID"]]
			if !haskey(charBPOTypes, t1BP["blueprintTypeID"])
				continue
			end
		end
		profit, decr = optimal_invention(ship)
		isFrigate = any(t2BP["activities"]["manufacturing"]["skills"]) do s
						itemTypes[s["typeID"]]["name"] == "Advanced Small Ship Construction"
					end
		if profit > 0.0
			number = isFrigate? 10 : 1
			push!(result, ["typeID"=>ship, "decryptor"=>decr, "profit"=>profit*number, "number"=>number])
		end
	end
	sort!(result; lt=(x,y)->x["profit"] < y["profit"], rev=true)
	return result
end

fmt_float(f::Float64) = @sprintf("%.2f", f)

function print_materials(materials, intermediates, installCost::Float64, inventCost::Float64, number::Int=1)
	resolve_item_prices(keys(materials))
	println("Materials needed:")
	totalPrice = 0
	for (matID, matAmount) in materials
		matAmount *= number
		price = itemPrices[matID]["sell"] * matAmount
		println(itemTypes[matID]["name"] * " => $matAmount ($(fmt_float(price)) isk)")
		totalPrice += price
	end
	installCost *= number
	println("Materials $(fmt_float(totalPrice)) + Install $(fmt_float(installCost)) + Invent $(fmt_float(inventCost)) = $(fmt_float(totalPrice+installCost+inventCost)) isk")
	
	if !isempty(intermediates)
		println("\nIntermediate products:")
		for (matID, matAmount) in intermediates
			matAmount *= number
			println(itemTypes[matID]["name"] * " => $matAmount")
		end
	end
end

function print_opt_row(i::Int, o)
	name = itemTypes[o["typeID"]]["name"]
	number = o["number"]
	if haskey(o, "profit")
		profit = "=> " * fmt_float(o["profit"])
	else
		profit = ""
	end	
	decryptor = o["decryptor"]["name"]
	println("$i. $name x $number $profit using $decryptor")
end

function print_optimal(opt::Array)
	for i = 1:length(opt)
		print_opt_row(i, opt[i])
	end
end

function print_plan(plan)
	println("\nMaterials to manufacture")
	materials = Dict{Int, Float64}()
	intermediates = Dict{Int, Float64}()
	installCost = 0.0
	inventionCost = 0.0
	totalSell = 0.0
	resolve_item_prices([entry["typeID"] for entry in plan])
	for i = 1:length(plan)
		entry = plan[i]
		print_opt_row(i, entry)
		productID = entry["typeID"]
		decr = entry["decryptor"]
		number = entry["number"]
		invItem, invCost = invention_result(productID, decr)
		mats, intermeds, instCost = break_materials(1.0, invItem)
		inventionCost += invCost * number
		installCost += instCost * number
		totalSell += itemPrices[productID]["sell"] * number
		add_materials!(materials, mats, number)
		add_materials!(intermediates, intermeds, number)
	end
	materialCost = material_cost(materials)
	totalProfit = totalSell - (inventionCost + installCost + materialCost)
	println("Expected profit $(fmt_float(totalProfit))\n")
	print_materials(materials, intermediates, installCost, inventionCost)
end

function find_optimal(checkBPOs::Bool = true)
	opt = optimal_invention(checkBPOs)
	print_optimal(opt)
	print("\nEnter rows to invent & manufacture: ")
	rows = map(int, split(strip(readline()), [' ', ','], false))
	plan = [opt[rows[i]] for i = 1:length(rows)]
	print_plan(plan)
end

function add_to_plan(product, decryptor, number, plan = Array(Any, 0))
	if isa(product, AbstractString)
		product = itemNames[product]
	end
	if isa(decryptor, AbstractString)
		decryptor = filter(d->contains(d["name"], decryptor), decryptors)[1]
	end
	push!(plan, {"typeID"=>product, "decryptor"=>decryptor, "number"=>number})
end

function init()
	global config = get_config()
	global charSkills = make_index(:typeID, get_char_skills())
	global charBlueprints = make_index(:itemID, get_char_blueprints())
	global skills = make_index(:typeID, get_skills())
	
	global services = get_services()
	global crestAuth = get_authorization()
	global regions = get_regions()
	global itemTypes = get_item_types()
	global itemNames = get_item_names()
	global itemCategories = get_item_categories()
	global marketGroups = get_market_groups()
	global marketPrices = get_market_prices()
	global industryFacilities = get_industry_facilities()
	global industrySystems = get_industry_systems()
	global assemblyLines = get_assembly_lines()
	global blueprints = get_blueprints()
	global decryptors = get_decryptors()

	process_market_groups()
	process_industry()
	process_blueprints()
end

init()
find_optimal()

end
