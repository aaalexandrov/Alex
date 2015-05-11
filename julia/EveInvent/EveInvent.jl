module EveInvent

import Requests
import HttpServer
import URIParser
import YAML
import JSON
import ODBC
import LightXML
import Dates
import DataFrames

include("crest.jl")
include("crestauth.jl")
include("apiv2.jl")

global config, appInfo
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

get_blueprints() = YAML.load_file("data/blueprints.yaml")

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

function import_decryptors()
	ODBC.connect("esb_DATADUMP")
	res = ODBC.query(
		"""
		select ta.typeID, typeName, attributeName, valueFloat
		from dbo.dgmTypeAttributes ta, dbo.dgmAttributeTypes at, dbo.invTypes it
		where ta.typeID in (select typeID 
						    from dbo.invTypes it, dbo.invMarketGroups mg
						    where it.marketGroupID=mg.marketGroupID 
							      and mg.marketGroupName='Decryptors') 
			  and ta.attributeID=at.attributeID
			  and ta.typeID = it.typeID
		order by typeID, ta.attributeID
		"""
	)
	# ODBC.DataFrames.writetable("data/Decryptors.csv", res)
	decID = -1
	decryptors = {}
	for row = 1:size(res, 1)
		if decID != res[row, :typeID]
			decID = res[row, :typeID]
			dec = Dict{String, Any}()
			dec["id"] = decID
			dec["name"] = res[row, :typeName]
			dec["attributes"] = Dict{String, Float64}()
			push!(decryptors, dec)
		end
		dec["attributes"][res[row, :attributeName]] = res[row, :valueFloat]
	end
	json_write("data/decryptors.json", decryptors)
end

get_decryptors() = json_read("data/decryptors.json")

function get_authorization() 
	global appInfo = json_read("appinfo.json")
	return request_access(get_service("authEndpoint"), appInfo)
end

function get_config()
	config = json_read("config.json")
	characters = get_api("Account/Characters"; auth=config["api"])["characters"]
	characterInd = indexin([config["charName"]], characters[:name])[1]
	config["charID"] = ["characterID"=>characters[characterInd, :characterID]]
	return config
end

get_char_skills() = get_api("Char/CharacterSheet"; params=config["charID"], auth=config["api"])["skills"]
get_char_assets() = get_api("Char/AssetList"; params=config["charID"], auth=config["api"])["assets"]
get_char_blueprints() = get_api("Char/Blueprints"; params=config["charID"], auth=config["api"])["blueprints"]

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
	#global blueprints = get_blueprints()
	#global shipInventable = get_inventable_blueprints(shipTypes)
	#global decryptors = get_decryptors()
end

end
