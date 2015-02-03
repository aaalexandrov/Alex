import FTFont

type Font
    font::FTFont.Font
    texture::Texture

    Font() = new()
end

isvalid(font::Font) = isdefined(font, :font) && isdefined(font, :texture) && isvalid(font.texture)

function init(font::Font, ftFont::FTFont.Font)
    @assert !isvalid(font)
    font.font = ftFont
    font.texture = 
end

function done(font::Font)
    if isvalid(font)
        done(font.texture)
    end
end
