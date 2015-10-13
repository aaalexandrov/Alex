const urlApi = "https://api.eveonline.com/"

api_to_filename(url::AbstractString) = replace(rsplit(url, '/', 2, false)[2], ".aspx", "")
api_cache_name(url::AbstractString) = "cache/" * api_to_filename(url)

const timeFormat = "y-m-d H:M:S"

function xml_read(fileName::AbstractString)
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
	return xroot
end

function xml_write(fileName::AbstractString, data)
    dirName = dirname(fileName)
    if !isdir(dirName)
        mkpath(dirName)
    end
	open(fileName, "w+") do s
		write(s, data)
	end
end

function check_time(fileName::AbstractString, xroot::LightXML.XMLElement)
	serverTime = LightXML.content(LightXML.find_element(xroot, "currentTime"))
	serverCached = LightXML.content(LightXML.find_element(xroot, "cachedUntil"))
	serverTimeF = Dates.datetime2unix(Dates.DateTime(serverTime, timeFormat))
	serverCachedF = Dates.datetime2unix(Dates.DateTime(serverCached, timeFormat))
	writeTimeF = mtime(fileName)
	validTimeF = writeTimeF + (serverCachedF - serverTimeF)
	if validTimeF <= time()
		return nothing
	end
	return xroot
end

xml_read_timed(fileName::AbstractString) = check_time(fileName, xml_read_timed(fileName))

function xml_join_attribute_values(xElem::LightXML.XMLElement, keyAttribs::Array) 
	values = map(k->LightXML.attribute(xElem, k), keyAttribs)
	if any(v->v==nothing, values)
		return nothing
	end
	return values
end

function xml_find_by_attribute(xElem::LightXML.XMLElement, key::Array, keyAttribs::Array)
	matches = LightXML.XMLElement[]
	for e in LightXML.child_elements(xElem)
		attribVals = xml_join_attribute_values(e, keyAttribs)
		if attribVals == key
			push!(matches, e)
		end
	end
	return matches
end

function add_dataframe_row!(df::DataFrames.DataFrame, keyAttribs::Array, values::Array)
	attrSyms = map(symbol, keyAttribs)
	newCols = setdiff(attrSyms, names(df))
	for c in newCols
		df[c] = DataFrames.DataArray(AbstractString, size(df, 1))
	end
	row = DataFrames.DataArray(AbstractString, size(df, 2))
	attrIndices = indexin(attrSyms, names(df))
	for i in 1:length(attrIndices)
		row[attrIndices[i]] = values[i]
	end
	push!(df, row)
end

function xml_find(xElem::LightXML.XMLElement, keyArr::Array, keyIndex::Int, results::DataFrames.DataFrame)
	keyAttr = LightXML.attribute(xElem, "key")
	keyAttribs = keyAttr != nothing? split(keyAttr, ",", false) : ["name"]
	key = keyArr[keyIndex]
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
	if keyIndex == length(keyArr) && isempty(key)
		attr = xml_join_attribute_values(xElem, keyAttribs)
		if attr != nothing
			add_dataframe_row!(results, keyAttribs, attr)
			return
		end
	end
	if key == "*"
		children = LightXML.child_elements(xElem)
	elseif matchAttribs
		children = xml_find_by_attribute(xElem, split(key, ","), keyAttribs)
	else
		children = LightXML.get_elements_by_tagname(xElem, key)
	end
	for child in children
		if keyIndex == length(keyArr)
			add_dataframe_row!(results, ["_content"], [LightXML.content(child)])
		else
			xml_find(child, keyArr, keyIndex+1, results)
		end
	end
end

function convert_column_type(arr::DataFrames.DataArray)
	typeConvert = [int, float64]
	types       = [Int, Float64]
	minInd = 1
	for e in arr
		if DataFrames.isna(e)
			continue
		end
		i = minInd
		while i <= length(typeConvert)
			try
				typeConvert[i](e)
				break
			catch
				minInd = i+1
				if minInd > length(typeConvert)
					return arr
				end
			end
			i += 1
		end
	end
	newArr = DataFrames.DataArray(types[minInd], length(arr))
	for i = 1:length(arr)
		if !DataFrames.isna(arr[i])
			newArr[i] = typeConvert[minInd](arr[i])
		end
	end
	return newArr
end

# Matched paths are slash-delimited. Elements are of the kind [attribNames][:][attribValuesOrTag]
# attribNames and attribValuesOrTag are comma-delimited lists of attribute names and values to match in the parent's children.
# If attribNames is omitted, then the value of the parent's key attribute is used for the names, or if the parent has no key attribute, 
# attribNames defaults to "name".
# attribValuesOrTag can be empty only for the last element, in which case a comma-separated list of the values of attributes in attribNames is returned.
# If there isn't a colon in the element, then attribValuesOrTag is a tag name to match. 
# If the last element is a tag, then the contents of the tagged node are returned.
# If attribValuesOrTag="*", then all child nodes are processed for matches. If attribNames="*", then all current node's attributes are selected 
# (in the order they appear in the node).

function xml_find(xElem::LightXML.XMLElement, path::AbstractString)
	results = DataFrames.DataFrame()
	xml_find(xElem, split(path, "/", false), 1, results)
	for colName in names(results)
		results[colName] = convert_column_type(results[colName])
	end
	return results
end	

global userAgent = "EveInvent Julia APIv2 library, greyalex2003@yahoo.com"
set_api_user_agent(agent::AbstractString) = (global userAgent = agent)

function get_api_url(url::AbstractString; params::Dict = Dict(), auth::Dict = Dict(), ignoreCache::Bool = false)
	fileName = api_cache_name(url)
	data = ignoreCache? nothing : xml_read(fileName)
	if data == nothing
		params = merge(params, auth)
		dataStr = Requests.format_query_str(params)
		headers = ["Content-Type"=>"application/x-www-form-urlencoded", 
				   "Content-Length"=>string(length(dataStr)),
				   "User-Agent"=>userAgent]
		info("POST to $url with data: $dataStr")
		resp = Requests.post(url; data=dataStr, headers=headers)
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

get_api(endpoint::AbstractString; params::Dict = Dict(), auth::Dict = Dict(), ignoreCache::Bool = false) = get_api_url(urlApi * endpoint * ".xml.aspx"; params=params, auth=auth, ignoreCache=ignoreCache)

