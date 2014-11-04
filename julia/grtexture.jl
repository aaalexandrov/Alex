type Texture <: AbstractTexture
	texture::GLuint
	id::Symbol

	Texture() = new(0)
end

isvalid(tex::Texture) = tex.texture != 0

import Images
import FixedPointNumbers: Ufixed8

function init(tex::Texture, texPath::String; id::Symbol = symbol(texPath))
	@assert !isvalid(tex)

	tex.id = id
	texture = GLuint[0]
	glGenTextures(1, texture)
	tex.texture = texture[1]

	img = Images.imread(texPath)
	w, h = size(img.data)
	data = img.data
	if eltype(data) != Images.RGBA{Ufixed8}
		data = convert(Array{Images.RGBA{Ufixed8}}, img.data)
	end
	glBindTexture(TEXTURE_2D, tex.texture)
	glTexImage2D(TEXTURE_2D, 0, RGBA, w, h, 0, RGBA, UNSIGNED_BYTE, data)
	glGenerateMipmap(TEXTURE_2D)
end

function done(tex::Texture)
	if isvalid(tex)
		texture = GLuint[tex.texture]
		glDeleteTextures(1, texture)
		tex.texture = 0
	end
end

function apply(tex::Texture, index::Int)
	@assert isvalid(tex)

	glActiveTexture(convert(GLenum, TEXTURE0 + index))
	glBindTexture(TEXTURE_2D, tex.texture)
end

# todo: add functions to set texture / sampler parameters

