type VertexLayout
	vao::GLuint

	VertexLayout() = new(0)
end

isvalid(vl::VertexLayout) = vl.vao != 0

function init(vl::VertexLayout, mesh::AbstractMesh)
	@assert !isvalid(vl)
	@assert mesh.vbo != 0 && mesh.ibo != 0

	vao = GLuint[0]
	glGenVertexArrays(1, vao)
	vl.vao = vao[1]

	glBindVertexArray(vl.vao)

	glBindBuffer(ARRAY_BUFFER, mesh.vbo)
	glBindBuffer(ELEMENT_ARRAY_BUFFER, mesh.ibo)

	initbuffers(mesh)

	layoutType = eltype(mesh.vertices)
	fieldOffsets = fieldoffsets(layoutType)

	for i = 1:length(layoutType.types)
		fieldType = layoutType.types[i]
		glType, elements = typeelements(fieldType)

		glEnableVertexAttribArray(i-1)
		glVertexAttribPointer(i-1, elements, glType, FALSE, sizeof(layoutType), convert(Ptr{Void}, fieldOffsets[i]))
	end

	glBindVertexArray(0)
end

function done(vl::VertexLayout)
	if isvalid(vl)
		vao = GLuint[vl.vao]
		glDeleteVertexArrays(1, vao)
		vl.vao = 0
	end
	nothing
end

function apply(vl::VertexLayout)
	@assert isvalid(vl)

	glBindVertexArray(vl.vao)
end

function typeelements(jlType::DataType)
	if isempty(jlType.names)
		# simple type
		return (jl2gltype(jlType), 1)
	else
		elType = nothing
		for t in jlType.types
			if elType == nothing
				elType = t
			elseif elType != t
				error("typeelements: A field with non-uniform types found")
			end
		end
		return (jl2gltype(elType), length(jlType.names))
	end
end


type Mesh <: AbstractMesh
	vbo::GLuint
	ibo::GLuint
	layout::VertexLayout
	vertices::Vector
	indices::Vector{Uint16}
	bound::Shapes.Shape
	id::Symbol

	Mesh() = new(0, 0, VertexLayout())
end

isvalid(mesh::Mesh) = mesh.vbo != 0 && mesh.ibo != 0 && isvalid(mesh.layout)

function init{T}(mesh::Mesh, vertices::Vector{T}, indices::Vector{Uint16}, vertex2point::Function = identity; id::Symbol = :mesh)
	@assert !isvalid(mesh)
	@assert !isempty(vertices)
	@assert !isempty(indices)

	mesh.id = id
	buffers = GLuint[0, 0]
	glGenBuffers(2, buffers)

	mesh.vbo = buffers[1]
	mesh.ibo = buffers[2]
	mesh.vertices = vertices
	mesh.indices = indices

	init(mesh.layout, mesh)
	initbound(mesh, vertex2point)
end

function initbuffers(mesh::Mesh)
	# buffers should have been bound at this point, we just set the contents
	glBufferData(ARRAY_BUFFER, sizeof(mesh.vertices), mesh.vertices, STATIC_DRAW)
	glBufferData(ELEMENT_ARRAY_BUFFER, sizeof(mesh.indices), mesh.indices, STATIC_DRAW)
end

function initbound(mesh::Mesh, vertex2point::Function)
	if vertex2point == identity
		return
	end
	p = vertex2point(mesh.vertices[1])
	sphere = Shapes.Sphere(p, 0)
	box = Shapes.AABB(p, p)
	for i = 2:length(mesh.vertices)
		p = vertex2point(mesh.vertices[i])
		Shapes.addpoint(sphere, p)
		Shapes.addpoint(box, p)
	end
	mesh.bound = Shapes.volume(box) < Shapes.volume(sphere) ? box : sphere
end

function done(mesh::Mesh)
	if isvalid(mesh)
		done(mesh.layout)

		buffers = GLuint[mesh.vbo, mesh.ibo]
		glDeleteBuffers(2, buffers)
		mesh.vbo = 0
		mesh.ibo = 0
	end
end

function apply(mesh::Mesh)
	@assert isvalid(mesh)

	# all state is applied through the VAO
	apply(mesh.layout)
end

function render(mesh::Mesh)
	apply(mesh)
	glDrawElements(primitivemode(mesh), indexcount(mesh), indextype(mesh), 0)
end

indextype(mesh::Mesh) = jl2gltype(eltype(mesh.indices))

primitivemode(mesh::Mesh) = TRIANGLES

indexcount(mesh::Mesh) = length(mesh.indices)
