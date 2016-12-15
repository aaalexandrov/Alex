module Match

typealias NTupleT{T, N} NTuple{N, T}

type Arrays{T}
	arrays::Vector{Vector{T}}
	values::Vector{Dict{T, Vector{Int}}}
	bestRows::Int
	matches::Vector{NTupleT{Int}}
	states::Vector{Task}
	invocations::Int
	lastResult::Bool

	Arrays(arrays::Vector{Vector{T}}) = new(
		arrays,
		[Dict{T, Vector{Int}}() for i=1:length(arrays)],
		mapreduce(a->length(a), +, arrays),
		Vector{NTuple{length(arrays), Int}}(),
		Task[],
		0)
end

Arrays{T}(arrays::Vector{Vector{T}}) = init(Arrays{T}(arrays))

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

function match{T}(arrays::Arrays{T}, next::NTupleT{Int} = ntuple(i->1, length(arrays.arrays))#= next::Vector{Int} = [1 for i=1:length(arrays.arrays)]=#, rowCount::Int = 0)
	arrays.invocations += 1
	len = length(arrays.arrays)
	bestRemaining = mapreduce(max, 1:len) do i
		length(arrays.arrays[i]) + 1 - next[i]
	end
	validMask = mapreduce(|, 1:len) do i
		next[i] <= length(arrays.arrays[i])? (1<<i) : 0
	end
	rowCount + bestRemaining >= arrays.bestRows && return false
	if validMask & (validMask - 1) == 0 # one or zero bits set
		arrays.bestRows = rowCount + bestRemaining
		empty!(arrays.matches)
		return true
	end
	found = false
	matching = Vector{Int}(len)
	matchArr = Vector{Int}(len)
	nextInd = Vector{Int}(len)
	for startInd = 1:len-1
		validMask & (1<<startInd) == 0 && continue
		val = arrays.arrays[startInd][next[startInd]]
		fill!(matching, -1)
		matching[startInd] = next[startInd]
		mask = 0
		for j = startInd+1:len
			validMask & (1<<j) == 0 && continue
			matching[j] = value_index(arrays.values[j], val, next[j])
			@assert matching[j] < 0 || arrays.arrays[j][matching[j]] == val
			if matching[j] > next[j] # match will skip elements, we want to try recursively with or without it
				mask |= 1<<j
			elseif matching[j] == next[j] # matches an element we'll try later, mark it as invalid
				validMask &= ~(1<<j)
			end
		end
		c = mask
		while true
			matchCount = -1
			rowsAdded = 0
			for i = 1:len
				if i>=startInd && (c&(1<<i)!=0 || matching[i]==next[i])
					matchArr[i] = matching[i]
					nextInd[i] = matchArr[i]+1
					rowsAdded += nextInd[i] - next[i]
					matchCount += 1
				else
					matchArr[i] = -1
					nextInd[i] = next[i]
				end
			end
			rowsAdded -= matchCount
			nextTup = (nextInd...)
			# produce(@task match(arrays, nextTup, rowCount+rowsAdded))
			if match(arrays, nextTup, rowCount+rowsAdded) # arrays.lastResult
				found = true
				if matchCount > 0
					push!(arrays.matches, (matchArr...))
				end
			end
			c == 0 && break
			# previous combination of masked bits
			c = (c - 1) & mask
		end
	end
	return found
end

function match{T}(arrays::Vector{Vector{T}})
	arrData = Arrays(arrays)
	match(arrData)
	#=
	push!(arrData.states, @task match(arrData))
	while !isempty(arrData.states)
		t = arrData.states[end]
		res = consume(t)
		if isa(res, Task)
			push!(arrData.states, res)
		else
			@assert isa(res, Bool)
			arrData.lastResult = res
			pop!(arrData.states)
		end
	end
	=#
	info(arrData.invocations)
	return arrData.matches, arrData.bestRows
end

end
