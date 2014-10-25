type Model
	mesh::Mesh
	material::Material
end

isvalid(model::Model) = isvalid(model.mesh) && isvalid(model.material)

function render(model::Model)
	@assert isvalid(model)

	apply(model.material)
	render(model.mesh)
end
