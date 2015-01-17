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
    glyphs::Dict{Char, Glyph}
    kerning::Dict{(Char, Char), Vec2{Float32}}
    bitmap::Array{Uint8, 2}

    Font(family, style, charWidth, charHeight, lineDistance, bmpWidth, bmpHeight) =
        new(family, style, Vec2{Float32}(charWidth, charHeight), lineDistance, Dict{Char, Glyph}(), Dict{(Char, Char), Vec2{Float32}}(), zeros(Uint8, bmpWidth, bmpHeight))
end

function addglyph(font::Font, char::Char, bmp::Array{Uint8, 2}, origin::Vec2{Float32}, advance::Vec2{Float32}, x, y, charHeight)
    @assert size(bmp, 2) < charHeight
    if x + size(bmp, 1) > size(font.bitmap, 1)
        x = 1
        y += charHeight
    end
    @assert x <= size(font.bitmap, 1)
    @assert y <= size(font.bitmap, 2)

    xmax = x + size(bmp, 1) - 1
    ymax = y + size(bmp, 2) - 1
    glyph = Glyph(char, rect(Int, x, y, xmax, ymax), origin, advance)
    font.glyphs[char] = glyph

    font.bitmap[x:xmax, y:ymax] = bmp
    x = xmax + 2

    return x, y
end

function addkerning(font::Font, c1::Char, c2::Char, distance::Vec2{Float32})
    @assert haskey(font.glyphs, c1) && haskey(font.glyphs, c2)
    font.kerning[(c1, c2)] = distance
end

function adding_done(font::Font)
    corner = one(Vec2{Int})
    for g in values(font.glyphs)
        corner = max(corner, g.box.max)
    end
    corner += one(Vec2{Int})
    if corner.x < size(font.bitmap, 1) || corner.size(font.bitmap, 2)
        font.bitmap = font.bitmap[1:corner.x, 1:corner.y]
    end
end

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

function font_bitmap_size(charWidth, charHeight, charCount)
    totalPixels = charWidth * charHeight * charCount
    width = ceil(sqrt(totalPixels) / charWidth) * charWidth
    height = ceil((totalPixels / width) / charHeight) * charHeight

    @assert (width / charWidth) * (height / charHeight) >= charCount

    return int(width), int(height)
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

function loadfont(faceName::String; cellWidth = 32, cellHeight = 32, faceIndex = 0, chars = '\u0000':'\u00ff', kerning::Bool = true)
    face = (FT_Face)[C_NULL]
    err = FT_New_Face(ftLib[1], faceName, int32(faceIndex), face)
    if err != 0
        info("Couldn't load font $faceName with error $err")
        return nothing
    end

    err = FT_Set_Pixel_Sizes(face[1], uint32(cellWidth), uint32(cellHeight))
    font = nothing
    if err != 0
        info("Couldn't set the pixel size for font $faceName with error $err")
    else
        faceRec = unsafe_load(face[1])

        charWidth, charHeight = max_glyph_size(face[1], faceRec, chars)

        # add a single pixel boundary between character boxes in the bitmap so it can be used for rendering without further processing
        charWidth += 1
        charHeight += 1

        bmpWidth, bmpHeight = font_bitmap_size(charWidth, charHeight, length(chars))

        lineDist = round(float32(faceRec.height) / faceRec.units_per_EM * cellHeight)

        font = Font(bytestring(faceRec.family_name),
                    bytestring(faceRec.style_name),
                    cellWidth,
                    cellHeight,
                    lineDist,
                    bmpWidth,
                    bmpHeight)

        # load glyphs
        x, y = 1, 1
        for c in chars
            err = FT_Load_Char(face[1], c, FT_LOAD_RENDER)
            @assert err == 0
            glyphRec = unsafe_load(faceRec.glyph)
            @assert glyphRec.format == FreeType.FT_GLYPH_FORMAT_BITMAP
            glyphBmp = glyph_bitmap(glyphRec.bitmap)

            x, y = addglyph(font, c, glyphBmp,
                            Vec2{Float32}(glyphRec.bitmap_left, glyphRec.bitmap_top),
                            Vec2{Float32}(glyphRec.advance.x / 64f0, glyphRec.advance.y / 64f0),
                            x, y, charHeight)
        end

        # query kerning info
        if faceRec.face_flags & FreeType.FT_FACE_FLAG_KERNING != 0
            kernDivisor = (faceRec.face_flags & FreeType.FT_FACE_FLAG_SCALABLE != 0) ? 64f0 : 1f0
            info
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
    return font
end

import Images

global font
function test()
    FTFont.init()

    global font = FTFont.loadfont("data/roboto-regular.ttf"; chars = ['\u0000':'\u00ff', 'А':'Я', 'а':'я'])
    Images.imwrite(Images.grayim(font.bitmap), "font.png")

    FTFont.done()
end

end
