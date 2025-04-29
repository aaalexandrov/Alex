using HTTP, Gumbo, AbstractTrees, DataFrames, CSV, Dates, Plots, Statistics

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

write_history(scrape_history())

movingfunc(f, g, n) = [f(g[max(1, i-n+1):i]) for i in 1:length(g)]
movingaverage(g, n) = movingfunc(mean, g, n)
movingmedian(g, n) = movingfunc(median, g, n)
movingmin(g, n) = movingfunc(minimum, g, n)
movingmax(g, n) = movingfunc(maximum, g, n)

normalize_series(g) = (g.-mean(g))./std(g)

function add_dayofweek!(data)
    data[!, "column-dayofweek"] = map(data[:, "column-date"]) do d Dates.dayofweek(Date(d)) end
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