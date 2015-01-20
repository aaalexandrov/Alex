import FTFont

type Font
    font::FTFont.Font
    texture::Texture

    Font() = new()
end

isvalid(font::Font) = isdefined(font, :font) && isdefined(font, :texture) && isvalid(font.texture)

function init(font::Font, ftFont::FTFont.Font)
    @assert !isvalid(font)
end

function done(font::Font)
    if isvalid(font)
    end
end
