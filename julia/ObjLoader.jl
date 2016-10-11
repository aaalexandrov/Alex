module ObjLoader

export load_obj

type StreamData
  id::Symbol
  dim::Int
  defaults::Vector
  data::Vector

  StreamData(id, elType, dim, defaults) = new(id, dim, defaults, Vector{elType}())
end

function add(stream::StreamData, v::Vector)
  @assert rem(length(stream.data), stream.dim) == 0
  append!(v, stream.defaults[length(v)]:stream.dim)
  if length(v) > stream.dim
    cols = getcolumns(stream)
    newData = vcat(reshape(stream.data, stream.dim, cols), repeat(defaults[stream.dim+1:length(v)], outer=[1, cols]))
    stream.data = reshape(newData, length(newData))
  end
  append!(stream.data, v)
  nothing
end

getlength(stream::StreamData) = div(length(stream.data), stream.dim)

getmatrix(stream::StreamData) = reshape(stream.data, stream.dim, getcolumns(stream))

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
    v1 = positions[:, face[i]]
    v2 = positions[:, face[i+1]]
    v3 = positions[:, face[i+2]]
    n = cross(v1-v2, v3-v2)
    len = norm(n)
    if len > eps(len)
      n /= len
      break
    end
  end
  n
end

averagenormals(normals::Matrix) = mean(normals, 2)

function fixnormals(streams::Vector{StreamData}, smoothGroups::Dict{Int, Vector})
  faces = get(streams, :faces)
end

function load_obj(io::IOStream)
  const faceDefault = [0, 0, 0]
  const streamIds = Dict("v" => :position, "vt" => :texCoord, "vn" => :normal)

  streams = Vector{StreamData}()
  smoothGroups = Dict{Int, Vector}()
  smoothGroupId = 0

  for l in eachline(io)
    l = strip(l)
    tokens = split(l)
    comment = findfirst(t->t[1] == '#', tokens)
    comment != 0 && tokens = tokens[1:comment-1]
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
        indVals = Matrix{Int}(length(indices), 1)
        for i = 1:length(indices)
          ind = isempty(indices[i]) ? 0 : parse(Int, indices[i])
          if ind < 0
            ind = lengths[i] + 1 + ind
          end
          indVals[i, 1] = ind
        end
        indVals
      end
      face = hcat(vals...)
      faces = getstream(streams, :faces, faceDefault)
      add(faces, face)
      faceNorm = facenormal(getmatrix(getstream(streams, :positions)), face)
      faceNormals = getstream(streams, :facenormals)
      add(faceNormals, faceNorm)
      if smoothGroupId != 0
        smoothGroup = get(()->Int[], smoothingGroups, smoothGroupId)
        push!(smoothGroup, getlength(faces))
      end
    else
      info("Skipping unsupported .obj line: $l")
    end
  end
end

end
