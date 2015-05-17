const evecURL = "http://api.eve-central.com/api/"

evec_filename(url::String) = "cache/" * replace(replace(url, evecURL, ""), urlReplaceKeys, c->urlReplace[c[1]]) * ".xml"

function get_evec(endpoint::String; query=Dict{String, Any}(), timeoutHours::Float64 = 6.0)
	url = evecURL * endpoint * "?" * Requests.format_query_str(query)
	fileName = evec_filename(url)
	data = mtime(fileName) + timeoutHours * 60 * 60 > time()? xml_read(fileName) : nothing
	if data == nothing
		headers = ["User-Agent"=>userAgent]
		info("GET from $url")
		resp = Requests.get(url; headers=headers)
		if resp.status != 200
			info("Response status: $(resp.status)")
			return nothing
		end
		xml_write(fileName, resp.data)
		xdoc = LightXML.parse_string(resp.data)
		data = LightXML.root(xdoc)
	end
	return data
end
