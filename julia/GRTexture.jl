type Texture <: AbstractTexture
	texture::GLuint
	id::Symbol
	renderer::Renderer

	Texture() = new(0)
end

isvalid(tex::Texture) = tex.texture != 0

import Images
import FixedPointNumbers: Ufixed8

function init(tex::Texture, renderer::Renderer, data::Array; id::Symbol = :texture)
	@assert !isvalid(tex)

	init_resource(tex, renderer, id)
	texture = GLuint[0]
	glGenTextures(1, texture)
	tex.texture = texture[1]

	w, h = size(data)
	pixelSize = sizeof(eltype(data))
	if pixelSize == 4
		pixelFormat = GL_RGBA
	elseif pixelSize == 1
		pixelFormat = GL_RED
	elseif pixelSize == 2
		pixelFormat = GL_RG
	elseif pixelSize == 3
		pixelFormat = GL_RGB
	else
		error("init(Texture): unsupported texture format")
	end
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
	glBindTexture(GL_TEXTURE_2D, tex.texture)
	glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, w, h, 0, pixelFormat, GL_UNSIGNED_BYTE, data)
	glGenerateMipmap(GL_TEXTURE_2D)
end

function init(tex::Texture, renderer::Renderer, texPath::String; id::Symbol = symbol(texPath))
	img = Images.imread(texPath)
	data = img.data
	pixelType = eltype(data)
	if pixelType != Images.RGBA{Ufixed8} && pixelType != Images.Gray{Ufixed8}
		data = convert(Array{Images.RGBA{Ufixed8}}, img.data)
	end
	init(tex, renderer, data; id = id)
end

function done(tex::Texture)
	if isvalid(tex)
		texture = GLuint[tex.texture]
		glDeleteTextures(1, texture)
		tex.texture = 0
		remove_renderer_resource(tex)
	end
end

function apply(tex::Texture, index::Int)
	@assert isvalid(tex)

	glActiveTexture(convert(GLenum, GL_TEXTURE0 + index))
	glBindTexture(GL_TEXTURE_2D, tex.texture)
end

# todo: add functions to set texture / sampler parameters
