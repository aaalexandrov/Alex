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

function getstream(streams::Vector{StreamData}, id::Symbol, defaults::Vector)
  ind = findfirst(s->s.id==id, streams)
  if ind == 0
    push!(streams, StreamData(id, eltype(defaults), 1, defaults))
    ind = length(streams)
  end
  streams[ind]
end

function load_obj(io::IOStream)
  const faceDefault = [0 0 0]
  const streamDefault = [0.0f0 0.0f0 0.0f0 1.0f0]
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
      stream = getstream(streams, streamIds[tokens[1]], streamDefault)
      vals = map(v->parse(Float32, v), tokens[2:end])
      add(stream, vals)
    elseif tokens[1] == "f"
      lengths = [getlength(getstream(streams, id, streamDefault)) for id in (:position, :texCoord, :normal)]
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
      faces = getstream(stream, :faces, faceDefault)
      add(faces, face)
    else
      info("Skipping unsupported .obj line: $l")
    end
  end
end

end
