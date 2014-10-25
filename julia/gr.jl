module GR

using OGL
import Shapes

abstract AbstractMesh
abstract AbstractTexture

include("grtypes.jl")

include("grmesh.jl")
include("gruniforms.jl")
include("grshader.jl")
include("grtexture.jl")
include("grstate.jl")
include("grmaterial.jl")
include("grmodel.jl")
include("grcamera.jl")


export Vec2, Vec3, Vec4, Matrix4
export VertexLayout, Mesh, Shader, Texture, Material, Model
export isvalid, init, done, apply, setuniform, getuniform, render

end
