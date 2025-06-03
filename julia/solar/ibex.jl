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

function save_model(mdl, path = "model.jld2")
    state = Flux.state(mdl)
    jldsave(path; state)
end

function load_model(path = "model.jld2")
    state = JLD2.load(path, "state")
    mdl = model_def()
    Flux.loadmodel!(mdl, state)
end

function dam_train_separate(data, model = [model_def() for o=0:23]; epochs = 1000, use_cuda = false, eta = 0.001, batchsize = 64)
    models = [dam_train(data, o, model[o+1]; epochs, use_cuda, eta, batchsize) for o=0:23]
    models
end

function plot_model_separate(model, data)
    training = dam_training_data(data, 0)
    xs = training[1][:, 1:24:end]
    prices = reshape(reduce(vcat, model[o+1](xs) for o=0:23), :)
    ys = reshape(training[2], :)
    @info stdmean(prices .- ys)
    plot([ys prices])
end

min_offset(mapping) = minimum(minimum(m[2]) for m in mapping)
max_offset(mapping) = maximum(maximum(m[2]) for m in mapping)
num_values(mapping) = sum(length(m[2]) for m in mapping)

function value_range(mapping, key)
    numBefore = 0
    for (k, i) in mapping
        if k == key
            return numBefore+1:numBefore+length(i)
        end
        numBefore += length(i)
    end
    0:-1
end    

function select_values(mapping, data; step=1, minOffs = min_offset(mapping), maxOffs = max_offset(mapping))
    mappedValues = num_values(mapping)
    firstElem = 1 + max(0, -minOffs)
    vals = Float32[]
    for row in firstElem:step:size(data, 1)-maxOffs
        for (col, offs) in mapping
            for o in offs
                push!(vals, data[row+o, col])
            end
        end
    end
    reshape(vals, mappedValues, :)
end

const Mapping = Vector{Pair}

@kwdef mutable struct NNModel
    paramsMapping::Mapping
    labelsMapping::Mapping
    dataStep::Int32 = 1
    epochs::Int32 = 1000
    batchsize::Int64 = 64
    eta::Float64 = 1e-3
    createFn::Function
    loss::Function = Flux.huber_loss
end

function training_data(nn::NNModel, data; step=nn.dataStep)
    minOffs = min(min_offset(nn.paramsMapping), min_offset(nn.labelsMapping))
    maxOffs = max(max_offset(nn.paramsMapping), max_offset(nn.labelsMapping))
    params = select_values(nn.paramsMapping, data; step, minOffs, maxOffs)
    labels = select_values(nn.labelsMapping, data; step, minOffs, maxOffs)
    params, labels
end

function train_nn(nn::NNModel, data, model = nn.createFn(nn); step = nn.dataStep, use_cuda=false, epochs=nn.epochs, eta=nn.eta, batchsize=nn.batchsize)
    to_device = identity
    if use_cuda
        CUDA.allowscalar(false)
        to_device = cu
    end

    training = training_data(nn, data; step)

    model_cu = model |> to_device
    optState = Flux.setup(Adam(eta), model_cu)
    @showprogress for epoch = 1:epochs
        loader = Flux.DataLoader(training, batchsize=batchsize, shuffle=true)
        for xy_cpu in loader
            x, y = xy_cpu |> to_device
            grads = Flux.gradient(model_cu) do m
                y_hat = m(x)
                nn.loss(y_hat, y)
            end
            Flux.update!(optState, model_cu, grads[1])
        end
    end
    model = model_cu |> cpu
    @info stdmean(model(training[1]) .- training[2])
    model
end

function plot_nn(model, nn::NNModel, data; step=num_values(nn.labelsMapping))
    training = training_data(nn, data; step)
    m = model(training[1])
    @info stdmean(m .- training[2])
    plot([reshape(training[2], :) reshape(m, :)])
end    

function predict_day(model, nn::NNModel, data, date)
    @assert length(nn.labelsMapping) == 1
    @assert nn.labelsMapping[1][2] == 0
    minOffs = min_offset(nn.paramsMapping)
    maxOffs = max_offset(nn.paramsMapping)
    dataPos = searchsortedfirst(data[:, "date"], date)
    dayInterval = dataPos:dataPos+23
    @assert all(d->d==date, data[dayInterval, "date"])
    outputRange = value_range(nn.paramsMapping, "price")
    @assert length(outputRange) == 24
    outputs = Float32[]
    perfect_pred = Float32[]
    while length(outputs) < 24
        params = select_values(nn.paramsMapping, data[dataPos+minOffs:dataPos+maxOffs, :]; minOffs, maxOffs)
        pred = model(params)
        append!(perfect_pred, pred)
        replaceRange = outputRange.stop-length(outputs)+1:outputRange.stop
        params[replaceRange] = outputs
        predicted = model(params)
        append!(outputs, predicted)
        dataPos += length(predicted)
    end
    [data[dayInterval, "price"] perfect_pred[1:24]  outputs[1:24]]
end

function around0_loss(ŷ, y; agg = mean, delta::Real = 1)
    δ = Flux.Losses.ofeltype(ŷ, delta)
    Flux.Losses._check_sizes(ŷ, y)
    abs_error = abs.(ŷ .- y) ./ (1 .+ abs.(y) ./ 10)

    agg(Flux.Losses._huber_metric.(abs_error, δ))
 end


dam_nn() = NNModel(
    paramsMapping = [
        "cloud_cover"=>0, 
        "global_tilted_irradiance"=>0, 
        "precipitation"=>0, 
        "temperature_2m"=>0, 
        "wind_speed_10m"=>0, 
        "workday"=>(-24, 0), 
        "phase_hour_sin"=>0, 
        "phase_hour_cos"=>0, 
        "phase_year_sin"=>0, 
        "phase_year_cos"=>0,
        "price"=>-24:-1,
    ],
    labelsMapping = [
        "price"=>0
    ],
    createFn = nn->begin 
        p = num_values(nn.paramsMapping)
        l = num_values(nn.labelsMapping)
        Chain(Dense(p=>p, selu), Dense(p=>l))
    end,
)

dam_1() = NNModel(
    paramsMapping = [
        "cloud_cover"=>(-24, 0), 
        "global_tilted_irradiance"=>(-24, 0), 
        "precipitation"=>(-24, 0), 
        "temperature_2m"=>(-24, 0), 
        "wind_speed_10m"=>(-24, 0), 
        "workday"=>(-24, 0), 
        "phase_hour_sin"=>0, 
        "phase_hour_cos"=>0, 
        "phase_year_sin"=>0, 
        "phase_year_cos"=>0,
        "price"=>-24,
    ],
    labelsMapping = [
        "price"=>0
    ],
    createFn = nn->begin 
        p = num_values(nn.paramsMapping)
        l = num_values(nn.labelsMapping)
        Chain(Dense(p=>p, selu), Dense(p=>l))
    end,
)

dam_1_offset(offs) = NNModel(
    paramsMapping = [
        "cloud_cover"=>-24:23, 
        "global_tilted_irradiance"=>-24:23, 
        "precipitation"=>-24:23, 
        "temperature_2m"=>-24:23, 
        "wind_speed_10m"=>-24:23, 
        "workday"=>-24:23, 
        "phase_hour_sin"=>offs, 
        "phase_hour_cos"=>offs, 
        "phase_year_sin"=>offs, 
        "phase_year_cos"=>offs,
        "price"=>-24:-1,
    ],
    labelsMapping = [
        "price"=>offs
    ],
    epochs = 500,
    createFn = nn->begin 
        p = num_values(nn.paramsMapping)
        l = num_values(nn.labelsMapping)
        Chain(Dense(p=>p, selu), Dense(p=>l))
    end,
    loss = around0_loss,
)

dam_24() = NNModel(
    paramsMapping = [
        "cloud_cover"=>0:23, 
        "global_tilted_irradiance"=>0:23, 
        "precipitation"=>0:23, 
        "temperature_2m"=>0:23, 
        "wind_speed_10m"=>0:23, 
        "workday"=>(-24, 0), 
        "phase_hour_sin"=>0, 
        "phase_hour_cos"=>0, 
        "phase_year_sin"=>0, 
        "phase_year_cos"=>0,
        "price"=>-24:-1,
    ],
    labelsMapping = [
        "price"=>0:23
    ],
    createFn = nn->begin 
        p = num_values(nn.paramsMapping)
        l = num_values(nn.labelsMapping)
        Chain(Dense(p=>p, selu), Dense(p=>l))
    end,
)

dam_24_1() = NNModel(
    paramsMapping = [
        "cloud_cover"=>-24:23, 
        "global_tilted_irradiance"=>-24:23, 
        "precipitation"=>-24:23, 
        "temperature_2m"=>-24:23, 
        "wind_speed_10m"=>-24:23, 
        "workday"=>-24:23, 
        "phase_hour_sin"=>0, 
        "phase_hour_cos"=>0, 
        "phase_year_sin"=>0, 
        "phase_year_cos"=>0,
        "price"=>-24:-1,
    ],
    labelsMapping = [
        "price"=>0:23
    ],
    createFn = nn->begin 
        p = num_values(nn.paramsMapping)
        l = num_values(nn.labelsMapping)
        Chain(Dense(p=>p, selu), Dense(p=>l))
    end,
)

dam_1_series() = [dam_1_offset(o) for o=0:23]

function train_series(nns, data, models = [nn.createFn(nn) for nn in nns]; step = length(nns), use_cuda=false, epochs=nns[1].epochs, eta=nns[1].eta, batchsize=nns[1].batchsize)
    for (nn,model) in zip(nns, models)
        train_nn(nn, data, model; step, use_cuda, epochs, eta, batchsize)
    end
    models
end    

function plot_series(models, nns, data)
    training = training_data(nns[end], data; step = length(nns))
    pred = reduce(vcat, model(params) for model in models, params in eachcol(training[1]))
    labels = data[25:end, nns[1].labelsMapping[1][1]]
    @info stdmean(pred .- labels)
    plot([labels pred])
end