using HTTP, Gumbo, AbstractTrees, DataFrames, CSV, Dates, Plots, Statistics, JSON, Flux, CUDA, ProgressMeter, JLD2

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

function replace_column!(fn, data, mapping)
    data[!, mapping[2]] = map(fn, data[:, mapping[1]])
    select!(data, Not(mapping[1]))
end

function convert_dam(dam)
    dh = copy(dam)
    if columnindex(dh, "column-date") > 0
        replace_column!(dh, "column-date"=>"date") do d
            if typeof(d) != Date
                d = Date(d)
            end
            d
        end
    end
    if columnindex(dh, "column-time_part") > 0
        replace_column!(dh, "column-time_part"=>"hour") do h 
            if typeof(h) == String
                h = parse(Int, h)
            end
            h - 1
        end
    end
    for n in names(dh)
        if startswith(n, "column-")
            replace_column!(dh, n=>n[length("column-")+1:end]) do s 
                if typeof(s) <: AbstractString
                    s = parse(Float64, s) 
                end
                s
            end
        end
    end
    dh
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
    parse_table(resp, "dam-history") |> convert_dam
end

function scrape_default()
    parse_table(HTTP.get("https://ibex.bg/dam-history.php"), "dam-history")
end

function write_history(prefix, data)
    fromDate = data[1, "date"]
    endDate = data[end, "date"]
    CSV.write("$prefix-$fromDate-to-$endDate.csv", data)
end

# write_history("dam-history", scrape_history())

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

const holidays = [
    Date("2024-01-01"),
    Date("2024-03-03"),
    Date("2024-05-01"),
    Date("2024-05-05"),
    Date("2024-05-06"),
    Date("2024-05-24"),
    Date("2024-09-06"),
    Date("2024-09-22"),
    Date("2024-12-24"),
    Date("2024-12-25"),
    Date("2024-12-26"),

    Date("2025-01-01"),
    Date("2025-03-03"),
    Date("2025-04-20"),
    Date("2025-05-01"),
    Date("2025-05-06"),
    Date("2025-05-24"),
    Date("2025-09-06"),
    Date("2025-09-22"),
    Date("2025-12-24"),
    Date("2025-12-25"),
    Date("2025-12-26"),
]

function is_workday(date)
    (dayofweek(date) in 1:5) && isempty(searchsorted(holidays, date))
end

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
    if fromDate < today - Dates.Month(1)
        weather = get_weather_history(min(endDate, Dates.today() - Dates.Day(3)), fromDate, lat, lon)
        fromDate = weather[end, "date"] + Dates.Day(1)
    end
    if fromDate <= endDate
        forecast = get_weather_forecast(max(0, endDate - Dates.today() + Dates.Day(1) |> Dates.days), Dates.today() - fromDate |> Dates.days, lat, lon)
        if !isnothing(weather)
            weather = vcat(weather, forecast)
        else
            weather = forecast
        end
    end
    weather
end

function average_data(dataArr)
    avg = copy(dataArr[1])
    for i = 2:length(dataArr)
        data = dataArr[i]
        for colName in names(data)
            col = data[!, colName]
            if eltype(col) == Float64
                avg[!, colName] += col
            end
        end
    end
    for colName in names(avg)
        col = avg[!, colName]
        if eltype(col) == Float64
            avg[!, colName] /= length(dataArr)
        end
    end
    avg
end

function get_weather_average(endDate = Dates.today(), fromDate = endDate - Dates.Month(3))
    dataArr = Dict{String, DataFrame}()
    for (loc, latLon) in weather_locations
        push!(dataArr, loc=>get_weather(endDate, fromDate, latLon[1], latLon[2]))
    end
    average_data(collect(values(dataArr)))
end

function join_histories(dam, weather)
    innerjoin(dam, weather, on=["date", "hour"])
end

function get_dam_with_weather(endDate = Dates.today() + Dates.Day(1), fromDate = endDate - Dates.Month(3) - Dates.Day(1), dam = nothing)
    if isnothing(dam)
        dam = scrape_history(endDate, fromDate)
    elseif typeof(dam) <: AbstractString
        dam = CSV.read(dam, DataFrame)
    end
    weather = get_weather_average(dam[end, "date"], dam[begin, "date"])
    join_histories(dam, weather)
end

function merge_csvs(prefix = "dam-weather")
    expr = Regex("$prefix.*\\.csv")
    files = filter(readdir()) do f
        !isnothing(match(expr, f))
    end
    frames = map(files) do f 
        CSV.read(f, DataFrame)
    end
    data = vcat(frames...)
    sort!(data, ["date", "hour"])
    unique!(data, ["date", "hour"])
    @assert(size(data, 1) % 24 == 0)
    data
end

# write_history("dam-weather", get_dam_with_weather())

movingfunc(f, g, n) = [f(g[max(1, i-n+1):i]) for i in 1:length(g)]
movingaverage(g, n) = movingfunc(mean, g, n)
movingmedian(g, n) = movingfunc(median, g, n)
movingmin(g, n) = movingfunc(minimum, g, n)
movingmax(g, n) = movingfunc(maximum, g, n)

function stdmean(xs)
    m = mean(xs)
    stdm(xs, m), m
end    

function normalize_series(g) 
    s, m = stdmean(g)
    (g.-m)./s
end

date_to_phase(date) = 2pi * (dayofyear(date) - 1) / daysinyear(date)

function add_extra_data(data, neg_threshold = 0, prepend_days = 1)
    data[!, "negative"] = data[!, "price"] .<= neg_threshold
    data[!, "positive"] = 1 .- data[!, "negative"]
    data[!, "workday"] = is_workday.(data[!, "date"])
    data[!, "phase_year_sin"] = sin.(date_to_phase.(data[!, "date"]))
    data[!, "phase_year_cos"] = cos.(date_to_phase.(data[!, "date"]))
    data[!, "phase_hour_sin"] = sin.(data[!, "hour"].*2pi./24)
    data[!, "phase_hour_cos"] = cos.(data[!, "hour"].*2pi./24)
    data
end

# coefs are divided by yearly mean for the values
distance_coefs = [
    ("workday", 10000.0),
    ("global_tilted_irradiance", 0.5897159647404511),
    ("temperature_2m", 2.4401360544217683),
    ("wind_speed_10m", 2.162790697674419),
    ("precipitation", 24.873015873015873),
    ("cloud_cover", 0.10504201680672232),
]

function distance_hour(h1, h2, coefs) 
    sum(abs(h1[coef[1]] - h2[coef[1]]) * coef[2] for coef in coefs)
end

function distance_day(day1, day2, coefs)
    @assert(size(day1, 1) == size(day2, 1) == 24)
    sum(distance_hour(day1[h, :], day2[h, :], coefs) for h = 1:24) *
        (1 + abs(Dates.days(day1[1, "date"] - day2[1, "date"])) / 30) # 30 days difference will multiply the distance by 2
end

function similar_days(data, day, numSimilar, coefs)
    distances = [(i, distance_day(day, data[i:i+23, :], coefs)) for i in 1:24:size(data, 1)]
    sort!(distances, lt=(a,b)->a[2]<b[2])
    map(distances[1:numSimilar]) do dist
        data[dist[1]:dist[1]+23, :]
    end
end    

function predict_day(data, forecastDay, coefs)
    price = data[:, "price"]
    avg = movingaverage(price, 168)
    day_delta = avg[end] - avg[end - 24]

    similar = average_data(similar_days(data, forecastDay, 4, coefs))
    targetAvg = mean(price[end-23:end]) + day_delta
    similarDelta = targetAvg - mean(similar[:, "price"])
    pred = similar[:, "price"] .+ similarDelta 
    @assert abs(mean(pred) - targetAvg) < 1e-6
    # correct the resulting function to extend to at least 0 around the interval where it crossed 0 in the original similar day
    for i = 1:length(pred)
        if similar[i, "price"] <= 0
            pred[i] = min(pred[i], 0)
        end
    end
    pred
end

function predict_days(data, days, coefs = distance_coefs)
    pred = Float64[]
    for d = size(data, 1)-24days+1:24:size(data, 1)
        data_slice = data[begin:d-1, :]
        forecastDay = data[d:d+23, :]
        append!(pred, predict_day(data_slice, forecastDay, coefs))
    end
    pred
end

function diff_predicted_days(data, days, coefs = distance_coefs)
    predict_days(data[begin:end, :], days, coefs) .- data[end-24days+1:end, "price"]
end

function optimize_coef(data, days, i, step, coefs)
    deviation = std(diff_predicted_days(data, days, coefs))
    found = false
    while true
        coefs[i] = (coefs[i][1], coefs[i][2]+step)
        dev = std(diff_predicted_days(data, days, coefs))
        if dev >= deviation
            coefs[i] = (coefs[i][1], coefs[i][2]-step)
            break
        end
        deviation = dev
        found = true
    end
    found
end

function optimize_coefs(data, days, step = 0.1, coefs = copy(distance_coefs))
    while true
        optimized = 0
        for i = 2:length(coefs)
            if optimize_coef(data, days, i, step, coefs) || optimize_coef(data, days, i, -step, coefs)
                optimized += 1
            end
        end
        if optimized == 0
            break
        end
    end
    coefs
end

function prepend_day(data)
    @assert data[1, "hour"] == 0
    cycle_date = data[1, "date"] - Dates.Day(1) + Dates.Year(1)
    cycle_rows = filter(r->r.date == cycle_date, data)
    #@assert size(cycle_rows, 1) == 24
    map!(d->d-Dates.Year(1), cycle_rows[!, "date"], cycle_rows[!, "date"])
    vcat(cycle_rows, data)
end

normalize_coefs = [
    "cloud_cover" => (33.63174884069838, 49.25521848602989),
    "global_tilted_irradiance" => (279.56952046078345, 197.3829637751787),
    "precipitation" => (0.17107508931322893, 0.05927550357374919),
    "temperature_2m" => (9.462730828618534, 12.754023513645224), 
    "wind_speed_10m" => (3.5381542287193444, 8.198834470435347),  
]

function normalize_coefs!(data)
    for (col, (div, sub)) in normalize_coefs
        data[!, col] = (data[!, col] .- sub) ./ div
    end
    data
end

function prev_day_indices(i)
    start = (div(i-1, 24) - 1) * 24 + 1
    start:start+23
end

function dam_training_data(data, offset)
    data = prepend_day(data)
    args = select(
        data[25+offset:end, :], 
        "cloud_cover", 
        "global_tilted_irradiance", 
        "precipitation", 
        "temperature_2m", 
        "wind_speed_10m", 
        "workday", 
        #"hour",
        "phase_hour_sin", 
        "phase_hour_cos", 
        "phase_year_sin", 
        "phase_year_cos",
    )

    normalize_coefs!(args)

    prev_workday = reduce(hcat, Float32.(data[i-24, "workday"]) for i=25+offset:size(data, 1))
    price_history = reduce(hcat, Float32.(data[i-24:i-1, "price"]) for i=25:size(data, 1)-offset)
    #price_history = reduce(hcat, Float32.(data[prev_day_indices(i), "price"]) for i=25:size(data, 1))

    labels = select(
        data[25+offset:end, :], 
        "price", 
        #"positive",
        "negative",
    )
    xs = reduce(hcat, Vector{Float32}.(eachrow(args)))
    ys = reduce(hcat, Vector{Float32}.(eachrow(labels)))
    vcat(xs, prev_workday, price_history), ys
end

#model_def() = Chain(Dense(34=>34, tanh), Dense(34=>2))
#model_def() = Chain(Dense(35=>35, tanh), Dense(35=>2))
model_def() = Chain(Dense(35=>35, selu), Dense(35=>35, tanh), Dense(35=>2))
#model_def() = Chain(Dense(34=>34, tanh), Dense(34=>2, tanh), Dense(2=>2))
#model_def() = Chain(Dense(33=>33, tanh), Dense(33=>2, tanh), Dense(2=>2))

function dam_train(data, offset, model = model_def(); epochs = 1000, use_cuda = false, eta = 0.001, batchsize = 64)
    to_device = identity
    if use_cuda
        CUDA.allowscalar(false)
        to_device = cu
    end
    training = dam_training_data(data, offset)

    model_cu = model |> to_device
    optState = Flux.setup(Adam(eta), model_cu)
    @showprogress for epoch = 1:epochs
        loader = Flux.DataLoader(training, batchsize=batchsize, shuffle=true)
        for xy_cpu in loader
            x, y = xy_cpu |> to_device
            grads = Flux.gradient(model_cu) do m
                y_hat = m(x)
                price_loss = Flux.huber_loss(y_hat[1,:], y[1,:])
                #price_loss = Flux.mse(y_hat[1,:], y[1,:])
                #neg_loss = Flux.logitbinarycrossentropy(y_hat[2:3,:], y[2:3,:])
                #neg_loss = Flux.huber_loss(y_hat[2:3,:], y[2:3,:])
                #neg_loss = Flux.huber_loss(y_hat[2,:], y[2,:])
                neg_loss = 0
                price_loss + neg_loss * 10
                #Flux.huber_loss(m(x), y)
                #Flux.msle(m(x), y)
                #Flux.mse(m(x), y)
                #Flux.binarycrossentropy(m(x), y)
                #Flux.logitcrossentropy(m(x), y)
                #Flux.logitbinarycrossentropy(m(x), y)
            end
            Flux.update!(optState, model_cu, grads[1])
        end
    end
    model = model_cu |> cpu
    @info stdmean(model(training[1]) .- training[2])
    model
end

# mm=dam_train(data, Chain(Dense(10=>10, tanh), Dense(10=>20, tanh), Dense(20=>10, tanh), Dense(10=>1)); epochs=10000)
# mm=dam_train(data, Chain(Dense(10=>10, tanh), Dense(10=>20, relu), Dense(20=>20, sigmoid), Dense(20=>10, softplus), Dense(10=>2)))

function plot_model(model, data, offset, row=1)
    training = dam_training_data(data, offset)
    m = model(training[1])
    @info stdmean(m[row,:] .- training[2][row,:])
    plot([training[2][row,:] m[row,:]])
end

function save_model(mdl, path = "model.jld2")
    state = Flux.state(mdl)
    jldsave(path; state)
end

function load_model(path = "model.jld2")
    state = JLD2.load(path, "state")
    mdl = model_def()
    Flux.loadmodel!(mdl, state)
end

function predict_day_nn(mdl, data, date)
    day = filter(r->r.date==date, data)
    weather = select(
        day, 
        "cloud_cover", 
        "global_tilted_irradiance", 
        "precipitation", 
        "temperature_2m", 
        "wind_speed_10m", 
        "workday", 
        "phase_hour_sin", 
        "phase_hour_cos", 
        "phase_year_sin", 
        "phase_year_cos",
    )
    normalize_coefs!(weather)
    weather = reduce(hcat, Vector{Float32}.(eachrow(weather)))

    prev_date = date - Dates.Day(1)
    prev_day = filter(r->r.date==prev_date, data)
    prices = Vector{Float32}(prev_day[:, "price"])

    recorded_prices = copy(prices)
    pred_prices = copy(recorded_prices)

    for i=1:24
        xs = vcat(weather[:,i], prices[end-23:end])
        price = mdl(xs)[1]
        push!(prices, price)

        xs = vcat(weather[:,i], recorded_prices[end-23:end])
        push!(pred_prices, mdl(xs)[1])
        push!(recorded_prices, day[i, "price"])
    end

    prices[end-23:end], Vector{Float32}(day[:, "price"]), pred_prices[end-23:end]
end    

function dam_training_data_24(data)
    data = prepend_day(data)
    args = select(
        data, 
        "cloud_cover", 
        "global_tilted_irradiance", 
        "precipitation", 
        "temperature_2m", 
        "wind_speed_10m", 
        # "workday", 
        # "phase_hour_sin", 
        # "phase_hour_cos", 
        # "phase_year_sin", 
        # "phase_year_cos",
    )
    times = select(
        data,
        "workday",
        #"phase_hour_sin", 
        #"phase_hour_cos", 
        "phase_year_sin", 
        "phase_year_cos",
    )

    normalize_coefs!(args)

    indRange = 25:24:size(data,1)-23
    weather = reduce(hcat, reshape(Matrix(Float32.(args[i:i+23, :])), 1, :)' for i in indRange)
    price_hist = reduce(hcat, Float32.(data[i-24:i-1, "price"]) for i in indRange)
    time = Matrix{Float32}(times[indRange, :])'
    workday_prev = Float32[data[i-24, "workday"] for i in indRange]'
    prices = reduce(hcat, Float32.(data[i:i+23, "price"]) for i in indRange)
    negatives = reduce(hcat, Float32.(data[i:i+23, "negative"]) for i in indRange)

    return vcat(weather, workday_prev, time, price_hist), vcat(prices, negatives)
end

#model_def_24() = Chain(Dense(172=>172, tanh), Dense(172 => 24, tanh), Flux.Scale(24))
#model_def_24() = Chain(Dense(172=>86, tanh), Dense(86 => 24))
#model_def_24() = Chain(Dense(172=>172, tanh), Dense(172=>172, tanh), Dense(172=>24))
#model_def_24() = Chain(Dense(172=>172, tanh), Dense(172=>172, tanh), Dense(172=>48))
#model_def_24() = Chain(Dense(264=>264, tanh), Dense(264=>264, tanh), Dense(264=>48))
#model_def_24() = Chain(Dense(264=>264, tanh), Dense(264=>48, sigmoid))
#model_def_24() = Chain(Dense(148=>148, tanh), Dense(148=>148, tanh), Dense(148=>48))
model_def_24() = Chain(Dense(148=>148, tanh), Dropout(0.2), Dense(148=>148, tanh), Dense(148=>148, relu), Dense(148=>48))
#model_def_24() = Chain(Dense(148=>148, tanh), Dense(148=>48))

function plot_model_24_tr(model, training)
    #days = reduce(hcat, training[1][:,i] for i=1:size(training[1], 2) if (i-1)%24==0)
    #prices = reduce(hcat, training[2][:,i] for i=1:size(training[2], 2) if (i-1)%24==0)
    days = training[1]
    prices = training[2]
    m = model(days)
    mLin = reshape(m[1:24,:], :)
    pLin = reshape(prices[1:24,:], :)
    @info stdmean(mLin .- pLin)
    plot([pLin mLin])
end

function plot_model_24(model, data)
    plot_model_24_tr(model, dam_training_data_24(data))
end    

function dam_train_24(data, model = model_def_24(); epochs = 1000, use_cuda = false, eta = 0.001, batchsize = 96)
    to_device = identity
    if use_cuda
        CUDA.allowscalar(false)
        to_device = cu
    end
    training = dam_training_data_24(data)

    model_cu = model |> to_device
    optState = Flux.setup(Adam(eta), model_cu)
    @showprogress for epoch = 1:epochs
        loader = Flux.DataLoader(training, batchsize=batchsize, shuffle=true)
        for xy_cpu in loader
            x, y = xy_cpu |> to_device
            grads = Flux.gradient(model_cu) do m
                y_hat = m(x)
                #Flux.huber_loss(y_hat, y)
                #divisors = map(x->max(x,0.1), minimum.(abs, eachcol(y[:,1:24])))
                #price_loss = reduce(+, sum.(eachcol((y_hat[:,1:24]-y[:,1:24]).^2 ./24))./divisors)
                #price_loss = Flux.huber_loss(y_hat[:,1:24]./max.(0.1f0, abs.(y[:,1:24])), sign.(y[:,1:24]))
                #price_loss = Flux.huber_loss(y_hat[:,1:24]./ map(x->max(0.001f0, abs(x)), y[:,1:24]), map(sign, y[:,1:24]))
                price_loss = Flux.mse(y_hat[:,1:24], y[:,1:24])
                neg_loss = Flux.huber_loss(y_hat[:,25:48], y[:,25:48])
                #neg_loss = 0
                price_loss + neg_loss * 10
            end
            Flux.update!(optState, model_cu, grads[1])
        end
    end
    model = model_cu |> cpu
    @info stdmean(model(training[1]) .- training[2])
    #plot_model_24_tr(model, training)
    model
end
