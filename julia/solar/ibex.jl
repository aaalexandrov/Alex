using HTTP, Gumbo, AbstractTrees, DataFrames

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

function scrape_table(url, targetId)
    resp = HTTP.get(url)
    resp_parsed = Gumbo.parsehtml(String(resp.body))
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
                    data[nrow(data), col] = text(ch)
                end
                false
            end
        end
        false
    end
    select(data, Not("#row"))
end

scrape_table("https://ibex.bg/dam-history.php", "dam-history")