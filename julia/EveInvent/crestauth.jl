type CrestAuthData
	requestState::String
	authorizationReceived::Bool
	serverClients::Array{HttpServer.Client, 1}
	serverTask::Task
	server::HttpServer.Server
	authorizationCode::String
	
	CrestAuthData(requestState::String) = new(requestState, false, Array(HttpServer.Client, 0))
end

function server_task(authData::CrestAuthData, req::HttpServer.Request, res::HttpServer.Response)
	resp = nothing
	try 
		query = HttpServer.parsequerystring(req.resource[3:end])
		if isa(query, Associative) && query["state"] == authData.requestState
			authData.authorizationCode = query["code"]
			resp = HttpServer.Response("Authorization code received, thank you")
		end
	catch e
		if !isa(e, String)
			rethrow()
		end
	end
	if resp == nothing
		resp = HttpServer.Response("Invalid request received")
	end
	return resp
end

function run_server(authData::CrestAuthData, port::Uint16)
	http = HttpServer.HttpHandler((req, res)->server_task(authData, req, res))
	http.events["connect"] = c->push!(authData.serverClients, c)
	http.events["close"] = c->filter!(sc->sc!=c, authData.serverClients)
	http.events["write"] = (c,r)->authData.authorizationReceived = isdefined(authData, :authorizationCode)
	authData.server = HttpServer.Server(http)
	authData.serverTask = @async run(authData.server, port)
end

function stop_server(authData::CrestAuthData)
	if isdefined(authData, :serverTask)
		try
			Base.throwto(authData.serverTask, InterruptException())
		end
		close(authData.server.http.sock)
		map(c->close(c.sock), authData.serverClients)
	end
end

function request_access_url(authData::CrestAuthData, appInfo::Dict)
	params = ["response_type" => "code", 
			  "redirect_uri" => appInfo["callbackURL"], 
			  "client_id" => appInfo["clientID"],
			  "scope" => appInfo["scope"],
			  "state" => authData.requestState]
	queryStr = Requests.format_query_str(params)
	return "https://login.eveonline.com/oauth/authorize/?$queryStr"
end

function open_browser(url::String)
    if OS_NAME == :Windows
	    url = replace(url, "&", "^&") # escape & on windows
	    run(`cmd /c start $url`)
    else
        spawn(`xdg-open "$url"`)
    end
end

function request_authorization_token(endpoint::String, appInfo::Dict, code::String, refresh::Bool)
	authStr = base64(appInfo["clientID"] * ":" * appInfo["secretKey"])
	if refresh
		params = ["grant_type" => "refresh_token",
				  "refresh_token" => code]
	else
		params = ["grant_type" => "authorization_code",
				  "code" => code]
	end
	dataStr = Requests.format_query_str(params)
	resp = Requests.post(endpoint; 
	                     data=dataStr, 
						 headers={"Authorization" => "Basic $authStr",
						          "Content-Type" => "application/x-www-form-urlencoded"})
	return JSON.parse(bytestring(resp.data))
end

function request_access(endpoint::String, appInfo::Dict) 
	auth = json_read("authorization.json")
	if auth == nothing
		authData = CrestAuthData("EveInvent$(rand(Uint))")
		uri = URIParser.URI(appInfo["callbackURL"])
		port = uri.port
		if port == 0
			port = uri.schema=="https"? 443 : 80
		end
		run_server(authData, port)
		open_browser(request_access_url(authData, appInfo))
		while !authData.authorizationReceived
			yield()
		end
		stop_server(authData)
		auth = request_authorization_token(endpoint, appInfo, authData.authorizationCode, false)
		json_write("authorization.json", auth)
	else
		auth = request_authorization_token(endpoint, appInfo, auth["refresh_token"], true)
	end
	return auth
end	
