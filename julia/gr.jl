module GR

using ModernGL
using OGLHelper

import Shapes

abstract AbstractRenderer
abstract Renderable
abstract Resource

function init_resource(r::Resource, renderer::AbstractRenderer, id::Symbol)
    r.renderer = renderer
    r.id = id
    add_renderer_resource(r)
end

getid(r::Resource) = r.id
getrenderer(r::Resource) = r.renderer

abstract AbstractMesh <: Resource
abstract AbstractTexture <: Resource

include("grtypes.jl")

include("grcamera.jl")
include("grstate.jl")
include("grrenderer.jl")
include("grmesh.jl")
include("gruniforms.jl")
include("grshader.jl")
include("grtexture.jl")
include("grmaterial.jl")
include("grmodel.jl")


export Vec2, Vec3, Vec4, Matrix4
export VertexLayout, Mesh, Shader, Texture, Material, Model
export isvalid, init, done, apply, setuniform, getuniform, render

end
