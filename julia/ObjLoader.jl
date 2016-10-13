module ObjLoader

import Base: isvalid

export load_obj, get_indexed, isvalid

const defaultIds = [:position, :texCoord, :normal]

typealias ITuple{N} NTuple{N, Int}

type ObjModel
  values::Vector{Matrix} # vector of data streams, each containing a type of values (positions, texture coordinates, normals)
  faces::Vector{Matrix{Int}} # vector of face data, each face containing indices inside values for each vertex
  valueIds::Vector{Symbol}

  ObjModel(ids, values, faces) = new(values, faces, copy(ids))
  ObjModel() = new([:position], [Matrix{Float32}(3, 0)], Matrix{Int}[])
end

matrixcols(v::Vector, dim::Int) = div(length(v), dim)
asmatrix(v::Vector, dim::Int) = reshape(v, dim, matrixcols(v, dim))

function value_matrix(values::Vector{Float32}, default::Vector{Float32})
  mat = asmatrix(values, length(default))
  rows = size(mat, 1)
  for r = rows:-1:1
    any(mat[r, i] != default[r] for i = 1:size(mat, 2)) && break
    rows = r
  end
  if rows < length(default)
    mat = mat[1:rows, :]
  end
  mat
end

load_obj(file::String) = open(io->load_obj(io), file)

function load_obj(io::IOStream)
  const defaults = [ [0f0, 0f0, 0f0, 1f0], [0f0, 0f0, 0f0], [0f0, 0f0, 0f0] ]

  values = [Float32[] for i = 1:3]
  smoothGroups = Dict{Int, Vector{Int}}()
  smoothGroupId = 0
  faces = Vector{Matrix{Int}}()

  for l in eachline(io)
    tokens = split(strip(l))
    comment = findfirst(t->t[1] == '#', tokens)
    if comment != 0
      tokens = tokens[1:comment-1]
    end
    isempty(tokens) && continue
    if tokens[1] == "s"
      smoothGroupId = tokens[2] == "off" ? 0 : parse(Int, tokens[2])
    elseif tokens[1] == "v" || tokens[1] == "vt" || tokens[1] == "vn"
      valInd = tokens[1] == "v" ? 1 : (tokens[1] == "vt" ? 2 : 3)
      vals = map(v->parse(Float32, v), tokens[2:end])
      append!(vals, defaults[valInd][length(vals)+1:end])
      append!(values[valInd], vals)
    elseif tokens[1] == "f"
      vals=map(tokens[2:end]) do t
        indices = split(t, '/')
        indVals = Matrix{Int}(3, 1)
        for i = 1:3
          ind = (i > length(indices) || isempty(indices[i])) ? 0 : parse(Int, indices[i])
          if ind < 0
            ind = matrixcols(values[i], length(defaults[i])) + 1 + ind
          end
          indVals[i, 1] = ind
        end
        indVals
      end
      face = hcat(vals...)
      push!(faces, face)
      if smoothGroupId != 0
        smoothGroup = get!(smoothGroups, smoothGroupId) do; Int[] end
        push!(smoothGroup, length(faces))
      end
    end
  end
  objValues = Matrix[value_matrix(values[1], [NaN32, NaN32, NaN32, 1.0f0]),
                     value_matrix(values[2], [NaN32, 0f0, 0f0]),
                     value_matrix(values[3], [NaN32, NaN32, NaN32])]
  model = ObjModel(defaultIds, objValues, faces)
  addnormals(model, Base.values(smoothGroups))
  delete_unused_values(model)
  return model
end

# ObjModel

function isvalid(model::ObjModel)
  dim = length(model.values)
  return length(model.valueIds) == dim && all(model.faces) do face
    size(face, 1) == dim
  end
end

function facenormal(model::ObjModel, faceInd::Int)
  posInd = findfirst(model.valueIds, :position)
  positions = model.values[posInd]
  face = model.faces[faceInd]
  n = zeros(Float32, 3)
  for i = 1:size(face, 2)-2
    v1 = positions[:, face[posInd, i]]
    v2 = positions[:, face[posInd, i+1]]
    v3 = positions[:, face[posInd, i+2]]
    n = cross(v1-v2, v2-v3)
    len = norm(n)
    if len > eps(len)
      n /= len
      break
    end
  end
  n
end

function facenormals(model::ObjModel)
  normals = Matrix{Float32}(3, length(model.faces))
  for i = 1:length(model.faces)
    normals[:, i] = facenormal(model, i)
  end
  normals
end

averagenormals(normals::Matrix) = mean(normals, 2) # maybe change to calculate axis of bounding cone?

function build_vertex_faces(model::ObjModel)
  posInd = findfirst(model.valueIds, :position)
  normalsInd = findfirst(model.valueIds, :normal)
  vertexFaces = Dict{Int, Vector{Int}}() # map between vertex indices and faces they are part of
  for faceInd = 1:length(model.faces)
    face = model.faces[faceInd]
    for i = 1:size(face, 2)
      if face[normalsInd, i] == 0 # face vertex has no specified normal
        faceArray = get!(vertexFaces, face[posInd, i]) do; Int[] end
        if isempty(faceArray) || faceArray[end] != faceInd # check if this vertex index wasn't specified already in this face's vertices
          push!(faceArray, faceInd)
        end
      end
    end
  end
  vertexFaces
end

function addnormals(model::ObjModel, smoothGroups = Vector{Int}[])
  posInd = findfirst(model.valueIds, :position)
  normalsInd = findfirst(model.valueIds, :normal)
  if normalsInd == 0
    normalsInd = add_values(model, :normal, 3)
  end
  vertexFaces = build_vertex_faces(model)
  faceNormals = facenormals(model)
  normals = model.values[normalsInd]
  normals = reshape(normals, length(normals))
  addedNormalIndices = Dict{ITuple, Int}()
  # add normals to vertices on faces inside smoothing groups
  for smoothFaces in smoothGroups, faceInd in smoothFaces
    face = model.faces[faceInd]
    for i = 1:size(face, 2)
      if face[normalsInd, i] == 0
        vertInd = face[posInd, i]
        faceArray = intersect(vertexFaces[vertInd], smoothFaces) # all faces in the current smoothing group that this vertex is part of
        faceIndexTuple = (faceArray...)
        face[normalsInd, i] = get!(addedNormalIndices, faceIndexTuple) do
          normalsToAverage = faceNormals[:, faceArray]
          normal = averagenormals(normalsToAverage)
          append!(normals, normal)
          return matrixcols(normals, 3)
        end
      end
    end
  end
  # add normals to faces outside smoothing groups
  for faceInd in 1:length(model.faces)
    face = model.faces[faceInd]
    for i = 1:size(face, 2)
      if face[normalsInd, i] == 0
        face[normalsInd, i] = get!(addedNormalIndices, (faceInd,)) do
          append!(normals, faceNormals[:, faceInd])
          return matrixcols(normals, 3)
        end
      end
    end
  end
  model.values[normalsInd] = asmatrix(normals, 3)
end

function append_face(indices::Vector{Int}, face::Vector{Int})
  # push the face as a fan in triangle list format
  for i = 1:length(face) - 2
    push!(indices, face[1], face[i+1], face[i+2])
  end
end

function get_indexed(model::ObjModel)
  const defaultVals = [0, 0, 0, 1]
  @assert length(model.values) == length(model.valueIds)
  generatedVertices = Dict{ITuple, Int}()
  streams = map(model.values) do v
    Vector{eltype(v)}()
  end
  indices = Int[]
  for face in model.faces
    @assert size(face, 1) == length(model.values)
    faceIndices = Int[]
    for i = 1:size(face, 2)
      indTuple = (face[:, i]...)
      vertInd = get!(generatedVertices, indTuple) do
        for s = 1:length(indTuple)
          value = indTuple[s] != 0 ?
                    model.values[s][:, indTuple[s]] :
                    defaultVals[1:size(model.values[s], 1)]
          append!(streams[s], value)
        end
        return matrixcols(streams[1], size(model.values[1], 1)) - 1
      end
      push!(faceIndices, vertInd)
    end
    append_face(indices, faceIndices)
  end
  vertices = Dict{Symbol, Array}(model.valueIds[s] => asmatrix(streams[s], size(model.values[s], 1)) for s = 1:length(model.values))
  return vertices, indices
end

function add_values(model::ObjModel, id::Symbol, dim::Int; elType::DataType = Float32)
  @assert isvalid(model)
  push!(model.values, Matrix{elType}(dim, 0))
  push!(model.valueIds, id)
  map!(model.faces) do face
    vcat(face, fill(0, 1, size(face, 2)))
  end
  return length(model.values)
end

function delete_values_at(model::ObjModel, ind::Int)
  @assert isvalid(model)
  deleteat!(model.values, ind)
  deleteat!(model.valueIds, ind)
  map!(model.faces) do face
    face[[i!=ind for i = 1:size(face, 1)], :]
  end
end

function delete_unused_values(model::ObjModel)
  i = 1
  while i <= length(model.values)
    if isempty(model.values[i])
      delete_values_at(model, i)
    else
      i += 1
    end
  end
end

# Model generation

function regularpoly(sides::Int, z::Float32 = 0f0)
	points = Matrix{Float32}(3, sides)
	for i in 1:sides
		ang = (i - 1) * 2pi / sides
		s = sin(ang)
		c = cos(ang)
		points[:, i] = [cos(ang), sin(ang), z]
	end

  face = [j for i=1:1, j=1:sides] # Matrix with a single row

	return points, face
end

function get_regular_poly(sides::Int, z::Float32 = 0f0)
  points, faces = regularpoly(sides, z)
  ObjModel([:position], [points], [faces])
end

end
