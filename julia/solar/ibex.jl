using HTTP, Gumbo, AbstractTrees, DataFrames, CSV, Dates, Plots, Statistics, JSON

function for_elem(predicate, root)
    for elem in PreOrderDFS(root)
        try
            if predicate(elem)
                return elem
            end
        catch
            #ignore things without tags
        end
    end
end

function parse_table(http_resp, targetId)
    resp_parsed = Gumbo.parsehtml(String(http_resp.body))
    table = for_elem(resp_parsed.root) do elem
        getattr(elem, "id") == targetId
    end
    tbody = for_elem(table) do elem 
        tag(elem) == :tbody
    end
    tbody = for_elem(tbody) do elem 
        tag(elem) == :tbody
    end
    data = DataFrame("#row"=>String[])
    for_elem(tbody) do elem
        if tag(elem) == :tr # table row
            push!(data, fill("", ncol(data)))
            data[nrow(data), "#row"] = string(nrow(data))
            for_elem(elem.children) do ch
                if tag(ch) == :td # table data
                    col = getattr(ch, "class")
                    if columnindex(data, col) <= 0
                        data[!, col] = fill("", nrow(data))
                    end
                    txt = Gumbo.text(ch)
                    if isnothing(tryparse(Float64, txt))
                        noComma = replace(txt, ","=>".")
                        if !isnothing(tryparse(Float64, noComma))
                            txt = noComma
                        end
                    end
                    data[nrow(data), col] = txt
                end
                false
            end
        end
        false
    end
    select(data, Not("#row"))
end

#currently history is limited to 3 months back
function scrape_history(endDate = Dates.today() + Dates.Day(1), fromDate = endDate - Dates.Month(3) - Dates.Day(1))
    resp = HTTP.post(
        "https://ibex.bg/dam-history.php"; 
        headers=Dict(
            "Content-Type"=>"application/x-www-form-urlencoded",
        ),
        body=string(HTTP.URI(;query=Dict(
            "fromDate"=>string(fromDate), 
            "endDate"=>string(endDate), 
            "but_search"=>"Search",
        )))[2:end]
    )
    parse_table(resp, "dam-history")
end

function scrape_default()
    parse_table(HTTP.get("https://ibex.bg/dam-history.php"), "dam-history")
end

function write_history(data)
    fromDate = data[1, "column-date"]
    endDate = data[end, "column-date"]
    CSV.write("dam-history-$fromDate-to-$endDate.csv", data)
end

# write_history(scrape_history())

const weather_locations = Dict(
    "Sofia"=>(42.6975, 23.3241),
    "Plovdiv"=>(42.15, 24.75),
    "Varna"=>(43.2167, 27.9167),
    "Burgas"=>(42.5061, 27.4678),
    "Pleven"=>(43.4167, 24.6167),
    "Ruse"=>(43.8487, 25.9534),
    "Vidin"=>(43.9916, 22.8824),
    "Sandanski"=>(41.5667, 23.2833),
    "Sopot"=>(42.65, 24.75),
)

function parse_weather(json, units)
    js = Dict{String, Any}()
    for (k,v) in json
        js[k] = if units[k] == "iso8601"
            map(v) do d parse(DateTime, d) end
        else
            map(Float64, v)
        end
    end
    DataFrame(js)
end    

tilt_from_lat(lat) = Int(round(lat)) - 7

function weather_forecast_uri(daysAhead, daysPast, lat, lon)
    HTTP.URI("https://api.open-meteo.com/v1/forecast"; query=Dict(
        "latitude"=>lat,
        "longitude"=>lon,
        "timezone"=>"Europe/Berlin",
        "hourly"=>"temperature_2m,wind_speed_10m,precipitation,cloud_cover,global_tilted_irradiance",
        "tilt"=>tilt_from_lat(lat),
        "past_days"=>daysPast,
        "forecast_days"=>daysAhead,
    ))
end

function weather_history_uri(endDate, fromDate, lat, lon)
    HTTP.URI("https://archive-api.open-meteo.com/v1/archive"; query=Dict(
        "latitude"=>lat,
        "longitude"=>lon,
        "timezone"=>"Europe/Berlin",
        "hourly"=>"temperature_2m,wind_speed_10m,precipitation,cloud_cover,global_tilted_irradiance",
        "tilt"=>tilt_from_lat(lat),
        "start_date"=>string(fromDate),
        "end_date"=>string(endDate),
    ))
end

function weather_table(uri)
    resp = HTTP.get(uri)
    json = JSON.parse(String(copy(resp.body)))
    data = parse_weather(json["hourly"], json["hourly_units"])
    data[!, "date"] = map(Date, data[:, "time"])
    data[!, "hour"] = map(hour, data[:, "time"])
    select!(data, Not("time"))
end

function get_weather_forecast(daysAhead = 7, daysPast = 7, lat = 42.6975, lon = 23.3241)
    weather_table(weather_forecast_uri(daysAhead, daysPast, lat, lon))
end

function get_weather_history(endDate = Dates.today() - Dates.Day(3), fromDate = endDate - Dates.Month(3) - Dates.Day(3), lat = 42.6975, lon = 23.3241)
    weather_table(weather_history_uri(endDate, fromDate, lat, lon))
end

function get_weather(endDate = Dates.today(), fromDate = endDate - Dates.Month(3), lat = 42.6975, lon = 23.3241)
    today = Dates.today()
    weather = nothing
    if fromDate < today - Dates.Month(3)
        weather = get_weather_history(min(endDate, Dates.today() - Dates.Day(3)), fromDate, lat, lon)
        fromDate = weather[end, "date"] + Dates.Day(1)
    end
    if fromDate < endDate
        forecast = get_weather_forecast(endDate - Dates.today() + Dates.Day(1) |> Dates.days, Dates.today() - fromDate |> Dates.days, lat, lon)
        if !isnothing(weather)
            weather = vcat(weather, forecast)
        else
            weather = forecast
        end
    end
    weather
end

function replace_column!(fn, data, mapping)
    data[!, mapping[2]] = map(fn, data[:, mapping[1]])
    select!(data, Not(mapping[1]))
end

function convert_dam(dam)
    dh = copy(dam)
    replace_column!(Date, dh, "column-date"=>"date")
    replace_column!(dh, "column-time_part"=>"hour") do h parse(Int, h)-1 end
    for n in names(dh)
        if eltype(dh[!, n]) == String && startswith(n, "column-")
            replace_column!(dh, n=>n[length("column-")+1:end]) do s parse(Float64, s) end
        end
    end
    dh
end

function join_histories(dam, weather)
    dh = convert_dam(dam)
    innerjoin(dh, weather, on=["date", "hour"])
end

movingfunc(f, g, n) = [f(g[max(1, i-n+1):i]) for i in 1:length(g)]
movingaverage(g, n) = movingfunc(mean, g, n)
movingmedian(g, n) = movingfunc(median, g, n)
movingmin(g, n) = movingfunc(minimum, g, n)
movingmax(g, n) = movingfunc(maximum, g, n)

normalize_series(g) = (g.-mean(g))./std(g)

function add_dayofweek!(data)
    data[!, "column-dayofweek"] = map(data[:, "column-date"]) do d 
        Dates.dayofweek(Date(d)) 
    end
end

function split_weekends(data)
    weekdays = filter(data) do r r["column-dayofweek"] in 1:5 end
    weekends = filter(data) do r r["column-dayofweek"] in 6:7 end
    weekdays, weekends
end

function predict_day(data)
    price = data[:, "column-price"]
    avg = movingaverage(price, 168)
    day_delta = avg[end] - avg[end - 24]
    price[end-23:end] .+ day_delta
end

function predict_days(data, days)
    pred = Float64[]
    for d = days-1:-1:0 
        data_slice = data[begin:end-24d, :]
        append!(pred, predict_day(data_slice))
    end
    pred
end

function diff_predicted_days(data, days)
    predict_days(data[begin:end-24, :], days) .- data[end-24days+1:end, "column-price"]
end    