import FTFont

type Font
    font::FTFont.Font
    model::Model
    vertexType::DataType
    textureUniform::Symbol

    Font = new()
end

isvalid(font::Font) = isdefined(font, :font) && isvalid(font.model)


function init(font::Font, ftFont::FTFont.Font, shader::Shader, vertexType::DataType, vertex2point::Function = identity; texureUniform::Symbol = :diffuseTexture, maxCharacters::Int = 4096)
    @assert isvalid(shader)

    fontName = fontname(font.font)
    texture = Texture()
    init(texture, shader.renderer, font.font.bitmap; id = symbol("Tex_" * fontName)

    material = Material(shader)
    setuniform(material, textureUniform, texture)

    mesh = Mesh()
    init(mesh, shader.renderer, Array(vertexType, maxCharacters * 4), zeros(Uint16, maxCharacters * 6), vertex2point; id = symbol("Mesh_" * fontName), usage = :dynamic)
    mesh.indexLength = 0

    font.font = ftFont
    font.vertexType = vertexType
    font.textureUniform = textureUniform
    font.model = Model(mesh, material)
end

function done(font::Font)
    if isvalid(font)
        texture = getuniform(font.model.material, font.textureUniform)
        done(texture)
        done(font.model.mesh)
    end
end

function drawchar(font::Font, pos::FTFont.Vec2{Float32}, bmp::Array{Uint8, 2}, box::FTFont.Rect{Int}, color)
    @assert font.model.mesh.indexLength < length(font.model.mesh.indices)
    baseIndex = font.model.mesh.indexLength
    baseVertex = baseIndex * 4 / 6
    indices = font.model.mesh.indices
    vertices = font.model.mesh.vertices

    vertType = eltype(vertices)
    vertices[baseIndex] = vertType()
end

function drawtext(font::Font, cursor::FTFont.TextCursor, s::String, color)
    @assert isvalid(font)
    startIndexLength = font.model.mesh.indexLength
    startVertexLength = startIndexLength * 4 / 6

    drawFunc = (pos::FTFont.Vec2{Float32}, bmp::Array{Uint8, 2}, box::FTFont.Rect{Int}, color)->drawchar(font, pos, bmp, box)
    drawtext(drawFunc, font.font, cursor, s)

    endIndexLength = font.model.mesh.indexLength
    endVertexLength = endIndexLength * 4 / 6
    updatebuffers(font.model.mesh, startVertexLength:endVertexLength, startIndexLength:endVertexLength)
end

function cleartext(font::Font)
    @assert isvalid(font)
    font.model.mesh.indexLength = 0
end
