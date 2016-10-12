module ObjLoader

export load_obj, get_indexed

const defaultIds = [:position, :texCoord, :normal]

type ObjModel
  values::Vector{Matrix} # vector of data streams, each containing a type of values (positions, texture coordinates, normals)
  faces::Vector{Matrix{Int}} # vector of face data, each face containing indices inside values for each vertex
  valueIds::Vector{Symbol}

  ObjModel(values, faces; ids = defaultIds) = new(values, faces, copy(ids))
end

type StreamData
  id::Symbol
  dim::Int
  defaults::Vector
  data::Vector

  StreamData(id, elType, dim, defaults) = new(id, dim, defaults, Vector{elType}())
end

typealias ITuple{N} NTuple{N, Int}

function add(stream::StreamData, v::Vector)
  @assert rem(length(stream.data), stream.dim) == 0
  append!(v, stream.defaults[length(v)+1:stream.dim])
  if length(v) > stream.dim
    cols = getlength(stream)
    newData = vcat(reshape(stream.data, stream.dim, cols), repeat(stream.defaults[stream.dim+1:length(v)], outer=[1, cols]))
    stream.data = reshape(newData, length(newData))
    stream.dim = length(v)
  end
  append!(stream.data, v)
  nothing
end

getlength(stream::StreamData) = div(length(stream.data), stream.dim)

getmatrix(stream::StreamData) = reshape(stream.data, stream.dim, getlength(stream))

load_obj(file::String) = open(io->load_obj(io), file)

const streamDefault = [0.0f0, 0.0f0, 0.0f0, 1.0f0]
function getstream(streams::Vector{StreamData}, id::Symbol, defaults::Vector = streamDefault)
  ind = findfirst(s->s.id==id, streams)
  if ind == 0
    push!(streams, StreamData(id, eltype(defaults), 1, defaults))
    ind = length(streams)
  end
  streams[ind]
end

function facenormal(positions::Matrix, face::Matrix)
  n = zeros(Float32, 3)
  for i = 1:size(face, 2)-2
    v1 = positions[:, face[1, i]]
    v2 = positions[:, face[1, i+1]]
    v3 = positions[:, face[1, i+2]]
    n = cross(v1-v2, v3-v2)
    len = norm(n)
    if len > eps(len)
      n /= len
      break
    end
  end
  n
end

averagenormals(normals::Matrix) = mean(normals, 2) # maybe change to calculate axis of bounding cone?

function build_vertex_faces(faces::Vector{Matrix{Int}})
  vertexFaces = Dict{Int, Vector{Int}}() # map between vertex indices and faces they are part of
  for faceInd = 1:length(faces)
    face = faces[faceInd]
    for i = 1:size(face, 2)
      if face[3, i] == 0 # face vertex has no specified normal
        faceArray = get!(vertexFaces, face[1, i]) do; Int[] end
        if isempty(faceArray) || faceArray[end] != faceInd # check if this vertex index wasn't specified already in this face's vertices
          push!(faceArray, faceInd)
        end
      end
    end
  end
  vertexFaces
end

function fixnormals(streams::Vector{StreamData}, smoothGroups::Dict{Int, Vector}, faces::Vector{Matrix{Int}})
  vertexFaces = build_vertex_faces(faces)
  faceNormals = getmatrix(getstream(streams, :facenormals))
  normals = getstream(streams, :normal)
  addedNormalIndices = Dict{ITuple, Int}()
  # add normals to vertices on faces inside smoothing groups
  for smoothFaces in values(smoothGroups), faceInd in smoothFaces
    face = faces[faceInd]
    for i = 1:size(face, 2)
      if face[3, i] == 0
        vertInd = face[1, i]
        faceArray = intersect(vertexFaces[vertInd], smoothFaces) # all faces in the current smoothing group that this vertex is part of
        faceIndexTuple = (faceArray...)
        face[3, i] = get!(addedNormalIndices, faceIndexTuple) do
          normalsToAverage = faceNormals[:, faceArray]
          normal = averagenormals(normalsToAverage)
          add(normals, reshape(normal, length(normal)))
          return getlength(normals)
        end
      end
    end
  end
  # add normals to faces outside smoothing groups
  for faceInd in 1:length(faces)
    face = faces[faceInd]
    for i = 1:size(face, 2)
      if face[3, i] == 0
        face[3, i] = get!(addedNormalIndices, (faceInd,)) do
          add(normals, faceNormals[:, faceInd])
          return getlength(normals)
        end
      end
    end
  end
end

function load_obj(io::IOStream)
  const streamIds = Dict("v" => :position, "vt" => :texCoord, "vn" => :normal)

  streams = Vector{StreamData}()
  smoothGroups = Dict{Int, Vector}()
  smoothGroupId = 0
  faces = Vector{Matrix{Int}}()

  for l in eachline(io)
    l = strip(l)
    tokens = split(l)
    comment = findfirst(t->t[1] == '#', tokens)
    if comment != 0
      tokens = tokens[1:comment-1]
    end
    isempty(tokens) && continue
    if tokens[1] == "s"
      smoothGroupId = tokens[2] == "off" ? 0 : parse(Int, tokens[2])
    elseif tokens[1] == "v" || tokens[1] == "vt" || tokens[1] == "vn"
      stream = getstream(streams, streamIds[tokens[1]])
      vals = map(v->parse(Float32, v), tokens[2:end])
      add(stream, vals)
    elseif tokens[1] == "f"
      lengths = [getlength(getstream(streams, id)) for id in (:position, :texCoord, :normal)]
      vals=map(tokens[2:end]) do t
        indices = split(t, '/')
        indVals = Matrix{Int}(3, 1)
        for i = 1:3
          ind = (i > length(indices) || isempty(indices[i])) ? 0 : parse(Int, indices[i])
          if ind < 0
            ind = lengths[i] + 1 + ind
          end
          indVals[i, 1] = ind
        end
        indVals
      end
      face = hcat(vals...)
      push!(faces, face)
      faceNorm = facenormal(getmatrix(getstream(streams, :position)), face)
      faceNormals = getstream(streams, :facenormals)
      add(faceNormals, faceNorm)
      if smoothGroupId != 0
        smoothGroup = get!(smoothGroups, smoothGroupId) do; Int[] end
        push!(smoothGroup, length(faces))
      end
    else
      # info("Skipping unsupported .obj line: $l")
    end
  end
  fixnormals(streams, smoothGroups, faces)
  values = [getmatrix(getstream(streams, s)) for s in defaultIds]
  model = ObjModel(values, faces; ids = defaultIds)
  i = 1
  while i <= length(model.values)
    if isempty(model.values[i])
      delete_values_at(model, i)
    else
      i += 1
    end
  end
  return model
end

# ObjModel

function append_face(indices::Vector{Int}, face::Vector{Int})
  # push the face as a fan in triangle list format
  for i = 1:length(face) - 2
    push!(indices, face[1], face[i+1], face[i+2])
  end
end

function get_indexed(model::ObjModel)
  @assert length(model.values) == length(model.valueIds)
  generatedVertices = Dict{ITuple, Int}()
  streams = Vector{StreamData}();
  for s = 1:length(model.valueIds)
    getstream(streams, model.valueIds[s])
  end
  indices = Int[]
  for face in model.faces
    @assert size(face, 1) == length(model.values)
    faceIndices = Int[]
    for i = 1:size(face, 2)
      indTuple = (face[:, i]...)
      vertInd = get!(generatedVertices, indTuple) do
        for s = 1:length(indTuple)
          stream = streams[s]
          value = indTuple[s] != 0 ?
                    model.values[s][:, indTuple[s]] :
                    stream.defaults[1:size(model.values[s], 1)]
          add(stream, value)
        end
        return getlength(streams[1]) - 1
      end
      push!(faceIndices, vertInd)
    end
    append_face(indices, faceIndices)
  end
  vertices = Dict(model.valueIds[s] => getmatrix(streams[s]) for s = 1:length(model.values))
  return vertices, indices
end

function delete_values_at(model::ObjModel, ind::Int)
  deleteat!(model.values, ind)
  deleteat!(model.valueIds, ind)
  for faceInd = 1:length(model.faces)
    model.faces[faceInd] = model.faces[faceInd][[i!=ind for i = 1:size(model.faces[faceInd], 1)], :]
  end
end

end
