const urlApi = "https://api.eveonline.com/"

api_to_filename(url::String) = replace(rsplit(url, '/', 2, false)[2], ".aspx", "")
api_cache_name(url::String) = "cache/" * api_to_filename(url)

const timeFormat = "y-m-d H:M:S"

function xml_read(fileName::String)
	local xmlStr, xdoc
	try 
		xmlStr = readall(fileName)
	catch e
		if isa(e, SystemError)
			return nothing
		else
			rethrow()
		end
	end
	try 
		xdoc = LightXML.parse_string(xmlStr)
	catch e
		if isa(e, LightXML.XMLParseError)
			return nothing
		else
			rethrow()
		end
	end
	xroot = LightXML.root(xdoc)
	serverTime = LightXML.content(LightXML.find_element(xroot, "currentTime"))
	serverCached = LightXML.content(LightXML.find_element(xroot, "cachedUntil"))
	serverTimeF = Dates.datetime2unix(Dates.DateTime(serverTime, timeFormat))
	serverCachedF = Dates.datetime2unix(Dates.DateTime(serverCached, timeFormat))
	writeTimeF = mtime(fileName)
	validTimeF = writeTimeF + (serverCachedF - serverTimeF)
	if validTimeF <= time()
		return nothing
	end
	return xmlStr
end

function xml_write(fileName::String, data)
	open(fileName, "w+") do s
		write(s, data)
	end
end

function xml_to_data_frames(xmlStr::String)
	xdoc = LightXML.parse_string(xmlStr)
	xroot = LightXML.root(xdoc)
	resNode = LightXML.find_element(xroot, "result")
	result = Dict{String, DataFrames.DataFrame}()
	for rowSet in LightXML.get_elements_by_tagname(resNode, "rowset")
		name = LightXML.attribute(rowSet, "name")
		columns = split(LightXML.attribute(rowSet, "columns"), ',', false)
		rows = LightXML.get_elements_by_tagname(rowSet, "row")
		frame = DataFrames.DataFrame([String for i=1:length(columns)], map(symbol, columns), length(rows))
		rn = 1
		for row in rows
			for col in columns
				frame[rn, symbol(col)] = LightXML.attribute(row, col)
			end
			rn += 1
		end
		result[name] = frame
	end
	return result
end

function xml_join_attribute_values(xElem::LightXML.XMLElement, keyAttribs::Array) 
	values = map(k->LightXML.attribute(xElem, k), keyAttribs)
	if any(v->v==nothing, values)
		return nothing
	end
	return join(values, ",")
end

function xml_find_by_attribute(xElem::LightXML.XMLElement, key::String, keyAttribs::Array)
	for e in LightXML.child_elements(xElem)
		attribVals = xml_join_attribute_values(e, keyAttribs)
		if attribVals == key
			return e
		end
	end
	return nothing
end

function xml_find(xElem::LightXML.XMLElement, keyArr::Array, results::Array{String, 1})
	keyAttr = LightXML.attribute(xElem, "key")
	keyAttribs = keyAttr != nothing? split(keyAttr, ",", false) : ["name"]
	key = keyArr[1]
	attrKey = split(key, ":")
	matchAttribs = length(attrKey) > 1
	if matchAttribs
		if !isempty(attrKey[1])
			if attrKey[1] == "*"
				keyAttribs = map(LightXML.name, LightXML.attributes(xElem))
			else
				keyAttribs = split(attrKey[1], ",")
			end
		end
		key = attrKey[2]
	end
	if length(keyArr) == 1 && isempty(key)
		attr = xml_join_attribute_values(xElem, keyAttribs)
		if attr != nothing
			push!(results, attr)
			return
		end
	end
	if key == "*"
		for child in LightXML.child_elements(xElem)
			xml_find(child, keyArr[2:end], results)
		end
		return
	end
	if matchAttribs
		child = xml_find_by_attribute(xElem, key, keyAttribs)
	else
		child = LightXML.find_element(xElem, key)
	end
	if child == nothing
		return
	end
	if length(keyArr) == 1
		push!(results, LightXML.content(child))
		return 
	end
	xml_find(child, keyArr[2:end], results)
end

# Matched paths are slash-delimited. Elements are of the kind [attribNames][:][attribValuesOrTag]
# attribNames and attribValuesOrTag are comma-delimited lists of attribute names and values to match in the parent's children.
# If attribNames is omitted, then the value of the parent's key attribute is used for the names, or if the parent has no key attribute, 
# attribNames defaults to "name".
# attribValuesOrTag can be empty only for the last element, in which case a comma-separated list of the values of attributes in attribNames is returned.
# If there isn't a colon in the element, then attribValuesOrTag is a tag name to match. 
# If the last element is a tag, then the contents of the tagged node are returned.
# If attribValuesOrTag="*", then all child nodes are processed for matches. If attribNames="*", then all current node's attributes are selected 
# (in the order they appear in the selected node).

function xml_find(xElem::LightXML.XMLElement, path::String)
	results = Array(String, 0)
	xml_find(xElem, split(path, "/", false), results)
	if search(path, '*') > 0
		return results
	else
		return isempty(results)? nothing : results[1]
	end
end	

function get_api_url(url::String; params::Dict = Dict(), auth::Dict = Dict())
	fileName = api_cache_name(url)
	data = xml_read(fileName)
	if data == nothing
		params = merge(params, auth)
		dataStr = Requests.format_query_str(params)
		headers = ["Content-Type"=>"application/x-www-form-urlencoded", 
				   "Content-Length"=>string(length(dataStr)),
				   "User-Agent"=>"EveInvent script, greyalex2003@yahoo.com"]
		info("POST to $url with data: $dataStr")
		resp = Requests.post(url; data=dataStr, headers=headers)
		if resp.status != 200
			return nothing
		end
		data = resp.data
		xml_write(fileName, data)
	end
	return xml_to_data_frames(data)
end

get_api(endpoint::String; params::Dict = Dict(), auth::Dict = Dict()) = get_api_url(urlApi * endpoint * ".xml.aspx"; params=params, auth=auth)

