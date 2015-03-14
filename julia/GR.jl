module GR

using ModernGL
using OGLHelper

import Shapes
import DevIL

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

include("GRTypes.jl")

include("GRCamera.jl")
include("GRState.jl")
include("GRRenderer.jl")
include("GRMesh.jl")
include("GRUniforms.jl")
include("GRShader.jl")
include("GRTexture.jl")
include("GRMaterial.jl")
include("GRModel.jl")
include("GRFont.jl")


export Vec2, Vec3, Vec4, Matrix4
export VertexLayout, Mesh, Shader, Texture, Material, Model
export isvalid, init, done, apply, setuniform, getuniform, render

end
