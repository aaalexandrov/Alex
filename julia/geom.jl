module Geom

using Math3D

reversetriangles(indices::Vector{Uint16}) = [indices[i + [0, 1, -1][i%3+1]] for i=1:length(indices)]

function unshareindices(indices::Vector{Uint16}, points::Matrix{Float32}, tounshare::Vector{Uint16})
	ptsCount = size(points, 2)
	pts = Array(Float32, 3, ptsCount + length(tounshare))
	pts[:, 1:ptsCount] = points
	ind = indices[:]
	iPt = ptsCount + 1
	for i = 1:length(tounshare)
		u = tounshare[i]
		used = findfirst(x->x!=u && ind[x]==ind[u], 1:length(ind))
		if used != 0
			pts[:, iPt] = pts[:, ind[u] + 1]
			ind[u] = iPt - 1
			iPt += 1
		end
	end
	return ind, pts[:, 1:iPt-1]
end

# returns an array of unique normal vectors and an array of indices into it
function trianglenormals(indices::Vector{Uint16}, points::Matrix{Float32})
	triCount = div(length(indices), 3)
	normals = Array(Float32, 3, triCount)
	iN = 1
	nIndices = Array(Uint16, triCount)
	const minCos = cos(pi/180)
	for i = 1:length(nIndices)
		p0 = points[:, indices[3(i-1)+1] + 1]
		p1 = points[:, indices[3(i-1)+2] + 1]
		p2 = points[:, indices[3(i-1)+3] + 1]
		n = normalize(cross(p1 - p0, p2 - p0))
		ind = findfirst(x->dot(n, normals[:, x]) > minCos, 1:iN-1)
		if ind == 0
			normals[:, iN] = n
			ind = iN
			iN += 1
		end
		nIndices[i] = ind
	end
	return nIndices, normals[:, 1:iN-1]
end

# for each point records an array of the indices of the indices in the index buffer that reference that point
function pointref(indices::Vector{Uint16}, points::Matrix{Float32})
	ptCount = size(points, 2)
	ref = [Uint16[] for i=1:ptCount]
	for i = 1:length(indices)
		push!(ref[indices[i] + 1], i)
	end
	return ref
end

function smoothnormals(indices::Vector{Uint16}, points::Matrix{Float32})
	triNIndices, triNormals = trianglenormals(indices, points)
	ptsRef = pointref(indices, points)
	normals = similar(points)
	for v = 1:size(normals, 2)
		# collect all normal indices for this vertex
		nIndices = Uint16[]
		for i in ptsRef[v]
			push!(nIndices, triNIndices[div(i-1, 3)+1])
		end

		# average out the unique normals
		n = Float32[0, 0, 0]
		for ni in unique(nIndices)
			n += triNormals[:, ni]
		end
		
		normals[:, v] = normalize(n)
	end
	return indices, points, normals
end

function facenormals(indices::Vector{Uint16}, points::Matrix{Float32})
	triNIndices, triNormals = trianglenormals(indices, points)
	ptsRef = pointref(indices, points)
	ptCount = sum(r->length(r), ptsRef)
	pts = Array(Float32, 3, ptCount)
	normals = similar(pts)
	ind = similar(indices)
	iPt = 1
	emitted = ((Uint16, Uint16) => Uint16)[]
	const minCos = cos(pi/180)
	for i = 1:length(indices)
		index = indices[i] + 1
		ni = triNIndices[div(i-1, 3)+1]
		ind[i] = get!(emitted, (index, ni), iPt)
		if ind[i] == iPt
			pts[:, iPt] = points[:, index]
			normals[:, iPt] = triNormals[:, ni]
			iPt += 1
		end
	end
	return ind, pts[:, 1:iPt-1], normals[:, 1:iPt-1]
end

function meshcat(meshes::(Vector{Uint16}, Matrix{Float32}, Matrix{Float32})...)
	indCount = 0
	ptsCount = 0
	for m in meshes
		ind, pts, norm = m
		indCount += length(ind)
		ptsCount += size(pts, 2)
	end
	indices = Array(Uint16, indCount)
	points = Array(Float32, 3, ptsCount)
	normals = similar(points)
	nextInd = 1
	nextPt = 1
	for m in meshes
		ind, pts, norm = m
		
		indices[nextInd:nextInd+length(ind)-1] = [i + nextPt - 1 for i in ind]
		nextInd += length(ind)
		
		points[:, nextPt:nextPt+size(pts, 2)-1] = pts
		normals[:, nextPt:nextPt+size(pts, 2)-1] = norm
		nextPt += size(pts, 2)
	end
	return indices, points, normals
end

function regularpoly(sides::Int, z::Float32 = 0f0)
	points = Array(Float32, 3, sides)
	for i in 1:sides
		ang = (i - 1) * 2pi / sides
		s = sin(ang)
		c = cos(ang)
		points[:, i] = [cos(ang), sin(ang), z]
	end
	
	indices = Array(Uint16, (sides - 2) * 3)
	for i in 0:sides-3
		indices[3i+1:3i+3] = [0, i+1, i+2]
	end
	
	return indices, points
end

fillcols(column::Vector, columns::Integer) = [column[r] for r=1:length(column), c=1:columns]

function prism(sides::Int, zMin::Float32 = -1f0, zMax::Float32 = 1f0; smooth::Bool = false)
	minInd, minPoints = regularpoly(sides, zMin)
	minInd = reversetriangles(minInd)
	maxInd, maxPoints = regularpoly(sides, zMax)

	sideInd = Array(Uint16, 6sides)
	sidePoints = Array(Float32, 3, 2sides)
	
	for i in 1:sides
		sidePoints[:, 2(i-1)+1] = minPoints[:, i]
		sidePoints[:, 2(i-1)+2] = maxPoints[:, i]
		
		nextInd = i % sides + 1
		sideInd[6(i-1)+1 : 6(i-1)+6] = [2(i-1), 2(nextInd-1), 2(nextInd-1)+1, 2(i-1), 2(nextInd-1)+1, 2(i-1)+1]
	end

	normFunc = smooth ? smoothnormals : facenormals
	minMesh = normFunc(minInd, minPoints)
	maxMesh = normFunc(maxInd, maxPoints)
	sideMesh = normFunc(sideInd, sidePoints)
	
	return meshcat(minMesh, maxMesh, sideMesh)
end

function pyramid(sides::Int, zMin::Float32 = 0f0, zMax::Float32 = 1f0; smooth::Bool = false)
	minInd, minPoints = regularpoly(sides, zMin)
	minInd = reversetriangles(minInd)

	sideInd = Array(Uint16, 3sides)
	sidePoints = Array(Float32, 3, 2sides)
	apex = [0, 0, zMax]

	for i = 1:sides
		sidePoints[:, 2(i-1)+1] = minPoints[:, i]
		sidePoints[:, 2(i-1)+2] = apex
		
		nextInd = i % sides + 1
		sideInd[3(i-1)+1:3(i-1)+3] = [2(i-1), 2(nextInd-1), 2(i-1)+1]
	end
	
	normFunc = smooth ? smoothnormals : facenormals
	minMesh = normFunc(minInd, minPoints)
	sideMesh = normFunc(sideInd, sidePoints)
	
	return meshcat(minMesh, sideMesh)
end

function sphere(segments::Int, rh::Float32 = 1f0, rv::Float32 = 1f0; smooth::Bool = true)
	vsegments = div(segments, 2) - 1
	
	ind = Array(Uint16, 6segments*vsegments)
	points = Array(Float32, 3, segments * vsegments + 2)
	ptsCount = size(points, 2)
	points[:, end] = Float32[0, 0, rv]
	points[:, end-1] = Float32[0, 0, -rv]
	
	nextInd = 1
	for x = 0:segments-1
		nextX = (x+1) % segments
		baseX = vsegments*x
		nextBaseX = vsegments*nextX
		angH = x*2pi/segments
		sinH = sin(angH)
		cosH = cos(angH)
		for y = 0:vsegments-1
			angV = (y+1)*pi/(vsegments+2)
			sinV = sin(angV)
			cosV = cos(angV)
			points[:, 1+baseX+y] = Float32[rh*sinV*cosH, rh*sinV*sinH, rv*cosV]
		end
		
		for y = 1:vsegments-1
			ind[nextInd:nextInd+5] = [baseX+y, nextBaseX+y, nextBaseX+y-1, baseX+y, nextBaseX+y-1, baseX+y-1] 
			nextInd += 6
		end
		
		ind[nextInd:nextInd+5] = [baseX, nextBaseX, ptsCount-1, nextBaseX+vsegments-1, baseX+vsegments-1, ptsCount-2]
		nextInd += 6
	end
	@assert nextInd == length(ind) + 1
	
	normFunc = smooth ? smoothnormals : facenormals
	return normFunc(ind, points)
end

end