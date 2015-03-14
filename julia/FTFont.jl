module FTFont

using FreeType
using Math2D

type Glyph
    char::Char
    box::Rect{Int}
    origin::Vec2{Float32}
    advance::Vec2{Float32}
end

type Font
    family::String
    style::String
    size::Vec2{Float32}
    lineDistance::Float32
    ascent::Float32
    descent::Float32
    glyphs::Dict{Char, Glyph}
    kerning::Dict{(Char, Char), Vec2{Float32}}
    bitmap::Array{Uint8, 2}
    fallbackGlyph::Glyph

    Font(family, style, sizeX, sizeY, lineDistance, ascent, descent, bmpWidth, bmpHeight) =
        new(family, style, Vec2{Float32}(sizeX, sizeY), lineDistance, ascent, descent, Dict{Char, Glyph}(), Dict{(Char, Char), Vec2{Float32}}(), zeros(Uint8, bmpWidth, bmpHeight))
end

Font(family, style, sizeX, sizeY, lineDistance, ascent, descent, maxCharWidth, maxCharHeight, charCount) =
    Font(family, style, sizeX, sizeY, lineDistance, ascent, descent, font_bitmap_size(maxCharWidth + 1, maxCharHeight + 1, charCount)...)

function font_bitmap_size(charWidth, charHeight, charCount)
    totalPixels = charWidth * charHeight * charCount
    width = ceil(sqrt(totalPixels) / charWidth) * charWidth
    height = ceil((totalPixels / width) / charHeight) * charHeight

    @assert (width / charWidth) * (height / charHeight) >= charCount

    return int(width), int(height)
end

type GlyphPosition
    x::Int
    y::Int
    yMax::Int

    GlyphPosition() = new(1, 1, 1)
end

function addglyph(font::Font, char::Char, bmp::Array{Uint8, 2}, origin::Vec2{Float32}, advance::Vec2{Float32}, pos::GlyphPosition)
    @assert size(bmp, 1) < size(font.bitmap, 1)
    @assert size(bmp, 2) < size(font.bitmap, 2)
    if pos.x + size(bmp, 1) > size(font.bitmap, 1)
        pos.x = 1
        pos.y = pos.yMax + 2
    end
    @assert pos.x <= size(font.bitmap, 1)
    @assert pos.y <= size(font.bitmap, 2)

    xmax = pos.x + size(bmp, 1) - 1
    ymax = pos.y + size(bmp, 2) - 1

    glyph = Glyph(char, rect(Int, pos.x, pos.y, xmax, ymax), origin, advance)
    font.glyphs[char] = glyph
    font.bitmap[pos.x:xmax, pos.y:ymax] = bmp
    # the first added glyph is used as a fallback when a glyph is missing while drawing
    if !isdefined(font, :fallbackGlyph)
        font.fallbackGlyph = glyph
    end

    pos.x = xmax + 2
    pos.yMax = max(pos.yMax, ymax)
end

function addkerning(font::Font, c1::Char, c2::Char, distance::Vec2{Float32})
    @assert haskey(font.glyphs, c1) && haskey(font.glyphs, c2)
    @assert !haskey(font.kerning, (c1, c2))
    font.kerning[(c1, c2)] = distance
end

function adding_done(font::Font)
    # maybe calculate the real ascent and descent from the glyph data as well, since the FreeType data can be off
    corner = one(Vec2{Int})
    for g in values(font.glyphs)
        corner = max(corner, g.box.max)
    end
    corner += one(Vec2{Int})
    if corner.x < size(font.bitmap, 1) || corner.y < size(font.bitmap, 2)
        font.bitmap = font.bitmap[1:corner.x, 1:corner.y]
    end
end

type TextCursor
    pos::Vec2{Float32}
    lastChar::Char

    TextCursor(x, y) = new(Vec2{Float32}(x, y), 0)
    TextCursor(cursor::TextCursor) = new(cursor.pos, cursor.lastChar)
end

function setpos(cursor::TextCursor, x, y)
    if x != cursor.pos.x || y != cursor.pos.y
        # we reset the last drawn character when we move the cursor so kerning only takes place when we draw continuous text
        cursor.pos = Vec2{Float32}(x, y)
        cursor.lastChar = 0
    end
end

function drawtext(drawRect::Function, font::Font, cursor::TextCursor, s::String)
    for c in s
        glyph = get(font.glyphs, c, font.fallbackGlyph)
        if cursor.lastChar != 0
            cursor.pos += get(font.kerning, (cursor.lastChar, c), zero(Vec2{Float32}))
        end
        drawRect(cursor.pos - glyph.origin, font.bitmap, glyph.box)
        cursor.pos += glyph.advance
        cursor.lastChar = c
    end
end

function textbox(font::Font, cursor::TextCursor, s::String)
    tempCursor = TextCursor(cursor)
    boxMax = rect(Float32)
    drawtext(font, tempCursor, s) do pos, bmp, box
        boxMax.min = min(boxMax.min, pos)
        boxMax.max = max(boxMax.max, pos + size(box))
    end
    return boxMax
end

fontname(font::Font) = "$(font.family)_$(font.style)($(font.size.x)x$(font.size.y))"

const ftLib = (FT_Library)[C_NULL]

function init()
    global ftLib
    @assert ftLib[1] == C_NULL
    err = FT_Init_FreeType(ftLib)
    return err == 0
end

function done()
    global ftLib
    @assert ftLib[1] != C_NULL
    err = FT_Done_FreeType(ftLib[1])
    ftLib[1] = C_NULL
    return err == 0
end

function max_glyph_size(face::FT_Face, faceRec::FreeType.FT_FaceRec, chars)
    width, height = 0, 0
    for c in chars
        err = FT_Load_Char(face, c, FT_LOAD_RENDER)
        @assert err == 0
        glyphRec = unsafe_load(faceRec.glyph)
        width = max(width, glyphRec.bitmap.width)
        height = max(height, glyphRec.bitmap.rows)
    end
    return width, height
end

function glyph_bitmap(bmpRec::FreeType.FT_Bitmap)
    @assert bmpRec.pixel_mode == FreeType.FT_PIXEL_MODE_GRAY
    bmp = Array(Uint8, bmpRec.width, bmpRec.rows)
    row = bmpRec.buffer
    if bmpRec.pitch < 0
        row -= bmpRec.pitch * (rbmpRec.rows - 1)
    end

    for r = 1:bmpRec.rows
        srcArray = pointer_to_array(row, bmpRec.width)
        bmp[:, r] = srcArray
        row += bmpRec.pitch
    end
    return bmp
end

function get_kerning(face::FT_Face, c1::Char, c2::Char, divisor::Float32)
    i1 = FT_Get_Char_Index(face, c1)
    i2 = FT_Get_Char_Index(face, c2)
    kernVec = Array(FreeType.FT_Vector, 1)
    err = FT_Get_Kerning(face, i1, i2, FreeType.FT_KERNING_DEFAULT, kernVec)
    if err != 0
        return zero(Vec2{Float})
    end
    return Vec2{Float32}(kernVec[1].x / divisor, kernVec[1].y / divisor)
end

function loadfont(faceName::String; sizeXY::(Real, Real) = (32, 32), faceIndex::Real = 0, chars = '\u0000':'\u00ff')
    face = (FT_Face)[C_NULL]
    err = FT_New_Face(ftLib[1], faceName, int32(faceIndex), face)
    if err != 0
        info("Couldn't load font $faceName with error $err")
        return nothing
    end

    err = FT_Set_Pixel_Sizes(face[1], uint32(sizeXY[1]), uint32(sizeXY[2]))
    font = nothing
    if err != 0
        info("Couldn't set the pixel size for font $faceName with error $err")
    else
        faceRec = unsafe_load(face[1])

        maxCharWidth, maxCharHeight = max_glyph_size(face[1], faceRec, chars)

        emScale = float32(sizeXY[2]) / faceRec.units_per_EM
        lineDist = round(faceRec.height * emScale)
        ascent = round(faceRec.ascender * emScale)
        descent = round(faceRec.descender * emScale)

        font = Font(bytestring(faceRec.family_name),
                    bytestring(faceRec.style_name),
                    sizeXY[1], sizeXY[2],
                    lineDist, ascent, descent,
                    maxCharWidth, maxCharHeight, length(chars))

        # load glyphs
        charPos = GlyphPosition()
        for c in chars
            err = FT_Load_Char(face[1], c, FT_LOAD_RENDER)
            @assert err == 0
            glyphRec = unsafe_load(faceRec.glyph)
            @assert glyphRec.format == FreeType.FT_GLYPH_FORMAT_BITMAP
            glyphBmp = glyph_bitmap(glyphRec.bitmap)

            addglyph(font, c, glyphBmp,
                     Vec2{Float32}(-glyphRec.bitmap_left, glyphRec.bitmap_top),
                     Vec2{Float32}(glyphRec.advance.x / 64f0, glyphRec.advance.y / 64f0),
                     charPos)
        end

        # query kerning info
        if faceRec.face_flags & FreeType.FT_FACE_FLAG_KERNING != 0
            kernDivisor = (faceRec.face_flags & FreeType.FT_FACE_FLAG_SCALABLE != 0) ? 64f0 : 1f0
            for c1 in chars, c2 in chars
                kerning = get_kerning(face[1], c1, c2, kernDivisor)
                if kerning != zero(Vec2{Float32})
                    addkerning(font, c1, c2, kerning)
                end
            end
        end

        adding_done(font)
    end

    err = FT_Done_Face(face[1])
    @assert err == 0
    return font
end

end
