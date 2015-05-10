global serverTasks = nothing
global server, serverClients = Array(HttpServer.Client, 1)
global currentRequestState = nothing
global authorizationCode, authorizationReceived

function server_task(req::HttpServer.Request, res::HttpServer.Response)
	global currentRequestState, authorizationCode
	resp = nothing
	try 
		query = HttpServer.parsequerystring(req.resource[3:end])
		if isa(query, Associative) && query["state"] == currentRequestState
			authorizationCode = query["code"]
			currentRequestState = nothing
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

function run_server()
	global serverClients, authorizationReceived, server, serverTask
	http = HttpServer.HttpHandler(server_task)
	http.events["connect"] = c->push!(serverClients, c)
	http.events["close"] = c->filter!(sc->sc!=c, serverClients)
	http.events["write"] = (c,r)->authorizationReceived = authorizationCode != nothing
	server = HttpServer.Server(http)
	empty!(serverClients)
	serverTask = @async run(server, 8321)
end

function stop_server()
	global serverTask, server, serverClients
	if serverTask != nothing
		try
			Base.throwto(serverTask, InterruptException())
		end
		close(server.http.sock)
		map(serverClients) do c
			flush(c.sock)
			close(c.sock)
		end
		empty!(serverClients)
		serverTask = nothing
		server = nothing
	end
end

const clientID = "13e88dc89c364981a0b1d074e5d6f1f3"
const secretKey = "cIzskc6lEZ2Hs7MNvqsbrh7VPtQIDsh1ZSfw5SLL"
const callbackURL = "http://localhost:8321"

function request_access_url()
	global currentRequestState = "EveInvent$(rand(Uint))"
	return "https://login.eveonline.com/oauth/authorize/?response_type=code&redirect_uri=$callbackURL&client_id=$clientID&scope=publicData&state=$currentRequestState"
end

function open_browser(url::String)
	url = replace(url, "&", "^&") # escape & on windows
	run(`cmd /c start $url`)
end

function request_authorization_token(endpoint::String, code::String, refresh::Bool)
	authData = base64("$clientID:$secretKey")
	if refresh
		dataStr="grant_type=refresh_token&refresh_token=$code"
	else
		dataStr="grant_type=authorization_code&code=$code"
	end
	resp = Requests.post(endpoint; 
	                     data=dataStr, 
						 headers={"Authorization" => "Basic $authData",
						          "Content-Type" => "application/x-www-form-urlencoded"})
	return JSON.parse(resp.data)
end

function request_access(endpoint::String) 
	auth = json_read("authorization.json")
	if auth == nothing
		global authorizationCode = nothing
		global authorizationReceived = false
		run_server()
		open_browser(request_access_url())
		while !authorizationReceived
			yield()
		end
		stop_server()
		auth = request_authorization_token(endpoint, authorizationCode, false)
		json_write("authorization.json", auth)
	else
		auth = request_authorization_token(endpoint, auth["refresh_token"], true)
	end
	return auth
end	
