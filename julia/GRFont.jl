import FTFont

type Font
    font::FTFont.Font
    model::Model
    vertexType::DataType
    textureUniform::Symbol

    Font() = new()
end

isvalid(font::Font) = isdefined(font, :font) && isvalid(font.model)


function init(font::Font, ftFont::FTFont.Font, shader::Shader, vertexType::DataType, vertex2point::Function = identity; textureUniform::Symbol = :diffuseTexture, maxCharacters::Int = 2048)
    @assert isvalid(shader)

    fontName = FTFont.fontname(ftFont)
    texture = Texture()
    init(texture, shader.renderer, ftFont.bitmap; id = symbol("Tex_" * fontName))

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

function drawchar(font::Font, pos::FTFont.Vec2{Float32}, bmp::Array{Uint8, 2}, box::FTFont.Rect{Int}, color::Color)
    @assert font.model.mesh.indexLength < length(font.model.mesh.indices)
    baseIndex = font.model.mesh.indexLength
    baseVertex = div(baseIndex * 4, 6)
    indices = font.model.mesh.indices
    vertices = font.model.mesh.vertices
    texWidth, texHeight = size(font.font.bitmap)
    boxF = FTFont.rect(Float32, box.min.x - 1, box.min.y - 1, box.max.x, box.max.y)
    boxFSize = size(boxF)

    vertType = eltype(vertices)
    vertices[baseVertex+1] = vertType(pos.x, pos.y, 0, color..., boxF.min.x / texWidth, boxF.min.y / texHeight)
    vertices[baseVertex+2] = vertType(pos.x, pos.y + boxFSize.y, 0, color..., boxF.min.x / texWidth, boxF.max.y / texHeight)
    vertices[baseVertex+3] = vertType(pos.x + boxFSize.x, pos.y, 0, color..., boxF.max.x / texWidth, boxF.min.y / texHeight)
    vertices[baseVertex+4] = vertType(pos.x + boxFSize.x, pos.y + boxFSize.y, 0, color..., boxF.max.x / texWidth, boxF.max.y / texHeight)

    indices[baseIndex+1] = baseVertex
    indices[baseIndex+2] = baseVertex + 1
    indices[baseIndex+3] = baseVertex + 3
    indices[baseIndex+4] = baseVertex
    indices[baseIndex+5] = baseVertex + 3
    indices[baseIndex+6] = baseVertex + 2

    font.model.mesh.indexLength += 6
end

function drawtext(font::Font, cursor::FTFont.TextCursor, s::String, color)
    @assert isvalid(font)
    startIndexLength = font.model.mesh.indexLength
    startVertexLength = div(startIndexLength * 4, 6)

    drawFunc = (pos::FTFont.Vec2{Float32}, bmp::Array{Uint8, 2}, box::FTFont.Rect{Int})->drawchar(font, pos, bmp, box, color)
    FTFont.drawtext(drawFunc, font.font, cursor, s)

    endIndexLength = font.model.mesh.indexLength
    endVertexLength = div(endIndexLength * 4, 6)
    updatebuffers(font.model.mesh, startVertexLength+1:endVertexLength, startIndexLength+1:endIndexLength)
end

function cleartext(font::Font)
    @assert isvalid(font)
    font.model.mesh.indexLength = 0
end
