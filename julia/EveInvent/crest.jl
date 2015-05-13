const urlCrest = "https://public-crest.eveonline.com/"

const urlReplaceKeys   = ['/', '?']
const urlReplaceValues = ['-', '.']
const urlReplace = Dict{Char, Char}(urlReplaceKeys, urlReplaceValues)
url_to_filename(url::String) = replace(replace(url, urlCrest[1:end-1], ""), urlReplaceKeys, c->urlReplace[c[1]]) * ".json"

function json_read(fileName::String)
	data = nothing
	try
		open(fileName, "r") do s
			data = JSON.parse(s)
		end
	catch e
		if !isa(e, SystemError)
			rethrow()
		end
	end
	return data
end

function json_write(fileName::String, data; indent::Int = 2)
	open(fileName, "w+") do s
		JSON.print(s, data, indent)
	end
end

function read_from_cache(url::String, timeout::Float64)
	fileName = "cache/" * url_to_filename(url)
	data = json_read(fileName)
	if time() - mtime(fileName) >= timeout
		data = nothing
	end
	return data
end

function write_to_cache(url::String, data)
	if !isdir("cache")
		mkdir("cache")
	end
	fileName = "cache/" * url_to_filename(url)
	json_write(fileName, data)
	nothing
end

function clear_cache(ext::String) 
	if isdir("cache")
		map(f->endswith(f, ext) && rm("cache/" * f), readdir("cache"))
	end
end

function get_crest(url::String, auth, timeoutHours::Float64 = inf(Float64))
	res = read_from_cache(url, timeoutHours * 60 * 60)
	if res != nothing
		return res
	end
	local queryRes
	orgUrl = url
	headers = Dict{String, String}()
	if auth != nothing
		headers["Authorization"] = auth["token_type"]*" "*auth["access_token"]
	end
	while true
		info("GET from $url")
		resp = Requests.get(url; headers=headers)
		if resp.status != 200
			info("Response status: $(resp.status)")
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
	write_to_cache(orgUrl, res)
	return res
end
