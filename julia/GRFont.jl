import FTFont

type Font
    font::FTFont.Font
    model::Model
    vertexType::DataType
    textureUniform::Symbol
    charCount::Int

    Font() = new()
end

isvalid(font::Font) = isdefined(font, :font) && isvalid(font.model)


function init(font::Font, ftFont::FTFont.Font, shader::Shader; positionFunc::Function = identity, textureUniform::Symbol = :diffuseTexture, maxCharacters::Int = 2048)
    @assert isvalid(shader)

    fontName = FTFont.fontname(ftFont)
    texture = Texture()
    init(texture, shader.renderer, ftFont.bitmap; id = symbol("Tex_" * fontName))

    material = Material(shader)
    setuniform(material, textureUniform, texture)
    setstate(material, AlphaBlendSrcAlpha())
    setstate(material, DepthStateDisabled())

    mesh = Mesh()
    init(mesh, shader.renderer, Array(shader.attribType, maxCharacters * 4), zeros(UInt16, maxCharacters * 6); positionFunc = positionFunc, id = symbol("Mesh_" * fontName), usage = :dynamic)
    mesh.indexLength = 0

    font.font = ftFont
    font.vertexType = shader.attribType
    font.textureUniform = textureUniform
    font.model = Model(mesh, material)
    font.charCount = 0
end

function done(font::Font)
    if isvalid(font)
        texture = getuniform(font.model.material, font.textureUniform)
        done(texture)
        done(font.model.mesh)
    end
end

function drawchar(font::Font, pos::FTFont.Vec2{Float32}, bmp::Array{UInt8, 2}, box::FTFont.Rect{Int}, color::Color)
    @assert font.model.mesh.indexLength < length(font.model.mesh.indices)
    baseIndex = font.charCount * 6
    baseVertex = font.charCount * 4
    indices = font.model.mesh.indices
    vertices = font.model.mesh.vertices
    texWidth, texHeight = size(font.font.bitmap)
    boxF = FTFont.rect(Float32, box.min.x - 1, box.min.y - 1, box.max.x, box.max.y)
    boxFSize = size(boxF)

    vertType = eltype(vertices)
	vecColor = Vec4(color...)
    vertices[baseVertex+1] = vertType(Vec3(pos.x,              pos.y,              0), vecColor, Vec2(boxF.min.x / texWidth, boxF.min.y / texHeight))
    vertices[baseVertex+2] = vertType(Vec3(pos.x,              pos.y + boxFSize.y, 0), vecColor, Vec2(boxF.min.x / texWidth, boxF.max.y / texHeight))
    vertices[baseVertex+3] = vertType(Vec3(pos.x + boxFSize.x, pos.y,              0), vecColor, Vec2(boxF.max.x / texWidth, boxF.min.y / texHeight))
    vertices[baseVertex+4] = vertType(Vec3(pos.x + boxFSize.x, pos.y + boxFSize.y, 0), vecColor, Vec2(boxF.max.x / texWidth, boxF.max.y / texHeight))

    indices[baseIndex+1] = baseVertex
    indices[baseIndex+2] = baseVertex + 1
    indices[baseIndex+3] = baseVertex + 3
    indices[baseIndex+4] = baseVertex
    indices[baseIndex+5] = baseVertex + 3
    indices[baseIndex+6] = baseVertex + 2

    font.charCount += 1
end

function drawtext(font::Font, cursor::FTFont.TextCursor, s::AbstractString, color)
    @assert isvalid(font)
    drawFunc = (pos::FTFont.Vec2{Float32}, bmp::Array{UInt8, 2}, box::FTFont.Rect{Int})->drawchar(font, pos, bmp, box, color)
    FTFont.drawtext(drawFunc, font.font, cursor, s)
end

function cleartext(font::Font)
    @assert isvalid(font)
    font.model.mesh.indexLength = 0
    font.charCount = 0
end

function updatemodel(font::Font)
    indexLength = font.model.mesh.indexLength
    @assert font.charCount * 6 >= indexLength
    if font.charCount * 6 > indexLength
        updatebuffers(font.model.mesh, div(indexLength*4, 6)+1:font.charCount*4, indexLength+1:font.charCount*6)
        font.model.mesh.indexLength = font.charCount*6
    end
end

function add(renderer::Renderer, font::Font)
    updatemodel(font)
    add(renderer, font.model)
end
