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
global services, regions, marketGroups
global shipTypes, blueprintTypes, blueprints, shipInventable
global decryptors

global charSkills, charBlueprints

global crestAuth = nothing

id_from_url(url::String) = int(rsplit(url, '/', 2, false)[end])

function server_version(serv)
	strVer = match(r"[0-9|.?]+", serv["serverVersion"])
	map(int, split(strVer.match, '.'))
end

function get_services() 
	oldServ = read_from_cache(urlCrest, inf(Float64))
	newServ = get_crest(urlCrest, crestAuth, 0.0)
	if oldServ == nothing || server_version(oldServ)[1:2] != server_version(newServ)[1:2]
		info("Server version differs from cached data, clearing cache")
		clear_cache(".json")
		write_to_cache(urlCrest, newServ)
	end
	return newServ
end

get_service(service::String) = services[service]["href"]

function get_regions()
	jsonRegions = get_crest(get_service("regions"), crestAuth)
	regions = Dict{String, Int}()
	for r in jsonRegions
		regions[r["name"]] = id_from_url(r["href"])
	end
	return regions
end

get_market_groups() = get_crest(get_service("marketGroups"), crestAuth)
get_market_group(group::String) = filter(g->g["name"]==group, marketGroups)[1]

function get_group_types(group::String)
	types = Dict{Int, String}()
	typeGroup = get_market_group(group)
	jTypes = get_crest(typeGroup["types"]["href"], crestAuth)
	for t in jTypes
		typ = t["type"]
		types[typ["id"]] = typ["name"]
	end
	return types
end

function access_path(o, p::Array)
	for i in p
		if !(isa(o, Associative) && haskey(o, i) || isa(o, AbstractArray) && 1 <= i <= length(o))
			return nothing
		end
		o = o[i]
	end
	return o
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

get_authorization() = request_access(get_service("authEndpoint"), config["appInfo"])

function get_config()
	config = json_read("config.json")
	config["appInfo"] = json_read("appinfo.json")
	set_api_user_agent(config["appInfo"]["userAgent"])
	characters = get_characters(config)
	characterInd = indexin([config["charName"]], characters[:name])[1]
	config["charID"] = ["characterID"=>characters[characterInd, :characterID]]
	return config
end

get_characters(config) = xml_find(get_api("Account/Characters"; auth=config["api"]), "result/:characters/*/*:")
get_char_skills() = xml_find(get_api("Char/CharacterSheet"; params=config["charID"], auth=config["api"]), "result/:skills/*/*:")
get_char_assets() = xml_find(get_api("Char/AssetList"; params=config["charID"], auth=config["api"]), "result/:assets/*/*:")
get_char_blueprints() = xml_find(get_api("Char/Blueprints"; params=config["charID"], auth=config["api"]), "result/:blueprints/*/*:")
get_skills() = xml_find(get_api("Eve/SkillTree"), "result/:skillGroups/*/:skills/*/*:")

function init()
	global config = get_config()
	global charSkills = get_char_skills()
	global charBlueprints = get_char_blueprints()
	
	global services = get_services()
	#global crestAuth = get_authorization()
	global regions = get_regions()
	global marketGroups = get_market_groups()
	global shipTypes = get_group_types("Ships")
	global blueprintTypes = get_group_types("Blueprints")
	global blueprints = get_blueprints()
	global shipInventable = get_inventable_blueprints(shipTypes)
	global decryptors = get_decryptors()
end

end
