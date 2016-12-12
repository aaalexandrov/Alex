module Match

type Arrays{T}
	arrays::Vector{Vector{T}}
	values::Vector{Dict{T, Vector{Int}}}
end

Arrays{T}(arrays::Vector{Vector{T}}) = init(Arrays{T}(arrays, [Dict{T, Vector{Int}}() for i=1:length(arrays)]))

function init{T}(arrays::Arrays{T})
	for a in 1:length(arrays.arrays)
		arr = arrays.arrays[a]
		val = arrays.values[a]
		for i in 1:length(arr)
			indices = get!(val, arr[i]) do; Int[] end
			push!(indices, i)
		end
	end
	arrays
end

function value_index{T}(values::Dict{T, Vector{Int}}, val::T, first::Int)
	!haskey(values, val) && return -1
	indices = values[val]
	i = searchsortedfirst(indices, first)
	i > length(indices) && return -1
	indices[i]
end

# return an array of tuples of matching indices, and a number of generated rows
function match{T}(arrays::Arrays{T}, next::Vector{Int} = [1 for i=1:length(arrays.arrays)])
	len = length(arrays.arrays)
	res = NTuple{len, Int}[]
	rows = 0
	for i = 1:len
		rows += length(arrays.arrays[i])+1-next[i]
	end
	if rows == 0
		return res, 0
	end
	rows = typemax(Int)
	for startInd = 1:len
		if next[startInd] > length(arrays.arrays[startInd])
			continue
		end
		val = arrays.arrays[startInd][next[startInd]]
		matching = fill(-1, len)
		matching[startInd] = next[startInd]
		mask = 0
		for j = startInd+1:len
			if next[j] > length(arrays.arrays[j])
				continue
			end
			matching[j] = value_index(arrays.values[j], val, next[j])
			if matching[j] >= 0
				mask |= 1<<j
				@assert arrays.arrays[j][matching[j]] == val
			end
		end
		c = 0
		while true
			matchArr = [
				if i<startInd
					-1
				elseif i==startInd
					next[startInd]
				else
					c & (1 << i) != 0? matching[i] : -1
				end

			  for i = 1:len]
			nextInd = [matchArr[i] < 0? next[i] : matchArr[i]+1 for i = 1:len]
			res1, rows1 = match(arrays, nextInd)
			rowsAdded = sum(nextInd - next) - count_ones(c)
			if rowsAdded+rows1 < rows
				rows = rowsAdded+rows1
				if c > 0
					res = [(matchArr...)]
				else
					res = res = NTuple{len, Int}[]
				end
				append!(res, res1)
			end

			if c >= mask
				break
			end
			# next combination of masked bits, leaving the rest 0
			# adding ~mask carries any 1s outside of the mask to the next masked bit
			c = (c + 1 + ~mask) & mask
		end
	end
	return res, rows
end

end
