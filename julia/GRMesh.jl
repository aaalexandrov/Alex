type VertexLayout
    vao::GLuint

    VertexLayout() = new(0)
end

isvalid(vl::VertexLayout) = vl.vao != 0

function init(vl::VertexLayout, mesh::AbstractMesh, usage::Symbol)
    @assert !isvalid(vl)
    @assert mesh.vbo != 0 && mesh.ibo != 0

    vao = GLuint[0]
    glGenVertexArrays(1, vao)
    vl.vao = vao[1]

    glBindVertexArray(vl.vao)

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo)

    initbuffers(mesh, usage)

    layoutType = eltype(mesh.vertices)
    fieldOffsets = fieldoffsets(layoutType)

    for i = 1:length(layoutType.types)
        fieldType = layoutType.types[i]
        glType, elements = typeelements(fieldType)

        glEnableVertexAttribArray(i-1)
        glVertexAttribPointer(i-1, elements, glType, GL_FALSE, sizeof(layoutType), convert(Ptr{Void}, fieldOffsets[i]))
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
        elType = Nothing
        for t in jlType.types
            if elType == Nothing
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
    indexLength::Int
    bound::Shapes.Shape
    id::Symbol
    renderer::Renderer

    Mesh() = new(0, 0, VertexLayout())
end

isvalid(mesh::Mesh) = mesh.vbo != 0 && mesh.ibo != 0 && isvalid(mesh.layout)

function init{T}(mesh::Mesh, renderer::Renderer, vertices::Vector{T}, indices::Vector{Uint16}, vertex2point::Function = identity; id::Symbol = :mesh, usage::Symbol = :static)
    @assert !isvalid(mesh)
    @assert !isempty(vertices)
    @assert !isempty(indices)

    init_resource(mesh, renderer, id)
    buffers = GLuint[0, 0]
    glGenBuffers(2, buffers)

    mesh.vbo = buffers[1]
    mesh.ibo = buffers[2]
    mesh.vertices = vertices
    mesh.indices = indices
    mesh.indexLength = length(indices)

    init(mesh.layout, mesh, usage)
    initbound(mesh, vertex2point)
end

function initbuffers(mesh::Mesh, usage::Symbol)
    if usage == :dynamic
        glUsage = GL_DYNAMIC_DRAW
    elseif usage == :stream
        glUsage = GL_STREAM_DRAW
    else
        @assert usage == :static
        glUsage = GL_STATIC_DRAW
    end
    # buffers should have been bound at this point, we just set the contents
    glBufferData(GL_ARRAY_BUFFER, sizeof(mesh.vertices), mesh.vertices, glUsage)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh.indices), mesh.indices, glUsage)
end

function initbound(mesh::Mesh, vertex2point::Function)
    if vertex2point == identity
        # mesh occupies all of space so it will never be clipped
        mesh.bound = Shapes.Space()
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

        remove_renderer_resource(mesh)
    end
end

function update_gl_buffer{T}(bufferType::GLenum, buffer::GLuint, data::Vector{T}, range::UnitRange)
    if !isempty(range)
        glBindBuffer(bufferType, buffer)
        glBufferSubData(bufferType, (range.start - 1) * sizeof(T), length(range) * sizeof(T), pointer(data, range.start))
    end
end

function updatebuffers(mesh::Mesh, vertexRange::UnitRange, indexRange::UnitRange)
    @assert isvalid(mesh)

    update_gl_buffer(GL_ARRAY_BUFFER, mesh.vbo, mesh.vertices, vertexRange)
    update_gl_buffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo, mesh.indices, indexRange)
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

primitivemode(mesh::Mesh) = GL_TRIANGLES

indexcount(mesh::Mesh) = mesh.indexLength
