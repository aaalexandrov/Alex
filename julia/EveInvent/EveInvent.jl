module EveInvent

import Requests
import YAML
import JSON

const urlCrest = "https://public-crest.eveonline.com/"

const b64urlKeys   = ['+', '/', '=']
const b64urlValues = ['-', '_', '%']
const b64url = Dict{Char, Char}(b64urlKeys, b64urlValues)
base64url(url::String) = replace(base64(url), b64urlKeys, c->b64url[c[1]])

function read_from_cache(url::String)
	fileName = "cache/" * base64url(url)
	data = nothing
	try
		data = JSON.parsefile(fileName)
	catch
	end
	return data
end

function write_to_cache(url::String, data)
	if !isdir("cache")
		mkdir("cache")
	end
	fileName = "cache/" * base64url(url)
	open(fileName, "w+") do s
		JSON.print(s, data, 2)
	end
	nothing
end

function get_crest(url::String)
	res = read_from_cache(url)
	if res != nothing
		return res
	end
	local queryRes
	while true
		resp = Requests.get(url)
		if resp.status != 200
			return nothing
		end
		queryRes = JSON.parse(resp.data)
		if res == nothing
			res = queryRes
		else
			append!(res["items"], queryRes["items"])
		end
		if !haskey(queryRes, "next")
			break
		end
		url = queryRes["next"]["href"]
	end
	@assert !haskey(queryRes, "totalcount") || queryRes["totalcount"] == length(res["items"])
	if haskey(res, "items")
		res = res["items"]
	end
	write_to_cache(url, res)
	return res
end

id_from_url(url::String) = int(rsplit(url, '/', 2, false)[end])

get_services() = get_crest(urlCrest)
global services, regions, marketGroups
global ships

get_service(service::String) = services[service]["href"]

function get_regions()
	jsonRegions = get_crest(get_service("regions"))
	regions = Dict{String, Int}()
	for r in jsonRegions
		regions[r["name"]] = id_from_url(r["href"])
	end
	return regions
end

get_market_groups() = get_crest(get_service("marketGroups"))
get_market_group(group::String) = filter(g->g["name"]==group, marketGroups)[1]

function get_group_types(group::String)
	types = Dict{String, Int}()
	typeGroup = get_market_group(group)
	jTypes = get_crest(typeGroup["types"]["href"])
	for t in jTypes
		typ = t["type"]
		types[typ["name"]] = typ["id"]
	end
	return types
end


function init()
	global services = get_services()
	global regions = get_regions()
	global marketGroups = get_market_groups()
	global ships = get_group_types("Ships")
end

end