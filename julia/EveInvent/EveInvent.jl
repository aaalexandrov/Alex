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
include("dataimp.jl")

global config
global services, regions, marketGroups, marketPrices, industrySystems
global shipTypes, blueprintTypes, blueprints, inventableProducts, productionProducts, shipInventable
global decryptors, skills

global charSkills, charBlueprints

global crestAuth = nothing

id_from_url(url::String) = int(rsplit(url, '/', 2, false)[end])

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

function make_index(key::Symbol, df::DataFrames.DataFrame)
	target = Dict{eltype(df[key]), Dict}()
	colNames = map(string, names(df))
	for i = 1:size(df, 1)
		target[df[i, key]] = [colNames[j] => df[i, j] for j=1:size(df, 2)]
	end
	return target
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
	make_index(jsonRegions, Dict{String, Int}()) do r
		r["name"], id_from_url(r["href"])
	end
end

get_market_groups() = get_crest(get_service(["marketGroups"]), crestAuth)
get_market_group(group::String) = filter(g->g["name"]==group, marketGroups)[1]

function get_group_types(group::String)
	typeGroup = get_market_group(group)
	jTypes = get_crest(typeGroup["types"]["href"], crestAuth)
	make_index(jTypes, Dict{Int, String}()) do t
		t["type"]["id"], t["type"]["name"]
	end
end

function get_market_prices() 
	priceArray = get_crest(get_service(["marketPrices"]), crestAuth, 6.0)
	make_index(priceArray, Dict{Int, Any}()) do p
		p["type"]["id"], p
	end
end

function get_industry_systems()
	systems = get_crest(get_service(["industry", "systems"]), crestAuth)
	make_index(systems, Dict{Int, Any}()) do s
		s["solarSystem"]["id"], s
	end
end

function get_decryptors()
	decr = json_read("data/decryptors.json")
	if decr == nothing
		info("Importing decryptors from static data dump...")
		import_decryptors()
		decr = json_read("data/decryptors.json")
	end
	return decr
end

function get_blueprints()
	bp = json_read("data/blueprints.json")
	if bp == nothing
		info("Importing data/blueprints.yaml")
		import_blueprints()
		bp = json_read("data/blueprints.json")
	end
	return [int(k)=>v for (k,v) in bp]
end	

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
	global shipTypes = get_group_types("Ships")
	global blueprintTypes = get_group_types("Blueprints")
end

get_product_id(blueprint, activity::String) = access_path(blueprint, ["activities", activity, "products", 1, "typeID"])

function get_inventable_blueprints(groupTypes::Dict{Int, String})
	groupPrints = Set{Int}()
	for (b,bv) in blueprints
		t2BlueprintID = get_product_id(bv, "invention")
		t2Blueprint = get(blueprints, t2BlueprintID, nothing)
		t2Product = get_product_id(t2Blueprint, "manufacturing")
		if haskey(groupTypes, t2Product) && haskey(blueprintTypes, b) # need to check if we're looking at a blueprint because of the t3 inventable research items
			push!(groupPrints, b)
		end
	end
	return groupPrints
end

function process_products(f::Function, blueprint, activity::String)
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
end

function production_amount(runs::Float32, baseQuantity::Float32, ME::Int, facilityMultiplier::Float32 = 1f0)
	materialModifier = facilityMultiplier * (1 - ME / 100f0)
	return max(runs, ceil(round(runs * baseQuantity * materialModifier, 2)))
end

function production_time(runs::Float32, baseProductionTime::Float32, TE::Int, skills::Array{Int}, facilityMultiplier::Float32 = 1f0)
	timeModifier = facilityMultiplier * (1 - TE / 100f0)
	skillModifier = prod(map(s->(1 - s / 100f0), skills))
	return baseProductionTime * timeModifier * skillModifier * runs
end

function base_job_cost(materials::Dict{Int, Float32})
	cost = 0f0
	for (matID, matAmount) in materials
		adjustedPrice = float32(marketPrices[matID]["adjustedPrice"])
		cost += matAmount * adjustedPrice
	end
	return cost
end

function production_fee(runs::Float32, baseJobCost::Float32, systemCostIndex::Float32)
	return runs * baseJobCost * systemCostIndex
end

function facility_tax(jobFee::Float32, facilityTaxRate::Float32)
	return jobFee * facilityTaxRate
end

function installation_cost(runs::Float32, materials::Dict{Int, Float32}, systemCostIndex::Float32, facilityTaxRate::Float32)
	baseCost = base_job_cost(materials)
	jobFee = production_fee(runs, baseCost, systemCostIndex)
	facilityTax = facility_tax(jobFee, facilityTaxRate)
	return jobFee + facilityTax
end

function is_encryption(skillID::Int)
	skillName = skills[skillID]["typeName"]
	return !isempty(search(skillName, "Encryption Methods"))
end

function invention_chance(baseChance::Float32, skills::Array, decryptorModifier::Float32)
	skillModifier = 1f0
	for s in skills
		skillID = s["typeID"]
		level = charSkills[skillID]["level"]
		skillModifier += is_encryption(skillID)? level/40f0 : level/30f0
	end
	return baseChance * skillModifier * decryptorModifier
end

function get_cost_index(system, activityName::String)
	filter(c->c["activityName"] == activityName, system["systemCostIndices"])[1]["costIndex"]
end

function blueprint_materials(runs::Float32, blueprintItem, systemID::Int)
	blueprint = blueprints[blueprintItem["typeID"]]
	ME = blueprintItem["materialEfficiency"]
	baseMaterials = blueprint["activities"]["manufacturing"]["materials"]
	materials = [m["typeID"]::Int => production_amount(runs, float32(m["quantity"]), ME) for m in baseMaterials]
	system = industrySystems[systemID]
	costIndex = get_cost_index(system, "Manufacturing")
end

function init()
	global config = get_config()
	global charSkills = make_index(:typeID, get_char_skills())
	global charBlueprints = make_index(:itemID, get_char_blueprints())
	global skills = make_index(:typeID, get_skills())
	
	global services = get_services()
	#global crestAuth = get_authorization()
	global regions = get_regions()
	global marketGroups = get_market_groups()
	global marketPrices = get_market_prices()
	global industrySystems = get_industry_systems()
	global blueprints = get_blueprints()
	global decryptors = get_decryptors()

	process_market_groups()
	process_blueprints()
end

end
