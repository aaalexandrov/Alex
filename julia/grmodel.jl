type Model <: Renderable
	mesh::Mesh
	material::Material
    transform::Matrix{Float32}
    boundDirty::Bool
    bound::Shapes.Shape{Float32}

    Model(mesh::Mesh, material::Material) = new(mesh, material, eye(Float32, 4), true)
end

isvalid(model::Model) = isvalid(model.mesh) && isvalid(model.material)

gettransform(model::Model) = model.transform
function settransform(model::Model, m::Matrix{Float32}) 
    @assert isvalid(model)
    model.transform[:, :] = m
    model.boundDirty = true
end

function getbound(model::Model)
    @assert isvalid(model)
    if model.boundDirty
        model.bound = Shapes.transform(model.mesh.bound, model.transform)
        model.boundDirty = false
    end
    model.bound
end

function render(model::Model, renderer::Union(Renderer, Nothing) = nothing)
    @assert isvalid(model)
    set_world_transform(model.material, model.transform)
    apply(model.material, renderer)
    render(model.mesh)
end