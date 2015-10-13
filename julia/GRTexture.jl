type Texture <: AbstractTexture
	texture::GLuint
	id::Symbol
	renderer::Renderer

	Texture() = new(0)
end

isvalid(tex::Texture) = tex.texture != 0

function init(tex::Texture, renderer::Renderer, data::Ptr{UInt8}, w::Integer, h::Integer, pixelFormat::GLenum; id::Symbol = :texture)
	@assert !isvalid(tex)

	init_resource(tex, renderer, id)
	texture = GLuint[0]
	glGenTextures(1, texture)
	tex.texture = texture[1]

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
	glBindTexture(GL_TEXTURE_2D, tex.texture)
	glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, w, h, 0, pixelFormat, GL_UNSIGNED_BYTE, data)
	glGenerateMipmap(GL_TEXTURE_2D)
end

const size2glFormat = [GL_RED, GL_RG, GL_RGB, GL_RGBA]

function init(tex::Texture, renderer::Renderer, data::Array; id::Symbol = :texture)
	w, h = size(data)
	pixelSize = sizeof(eltype(data))
	pixelFormat = size2glFormat[pixelSize]
	init(tex, renderer, pointer(data), w, h, pixelFormat; id = id)
end

# todo: add support for DXT textures

const il2glFormat =
	Dict([
		(DevIL.IL_RGBA, GL_RGBA),
		(DevIL.IL_BGRA, GL_BGRA),
		(DevIL.IL_RGB, GL_RGB),
		(DevIL.IL_BGR, GL_BGR),
		(DevIL.IL_LUMINANCE_ALPHA, GL_RG),
		(DevIL.IL_LUMINANCE, GL_RED),
		(DevIL.IL_ALPHA, GL_RED)
	])

function init(tex::Texture, renderer::Renderer, texPath::AbstractString; id::Symbol = symbol(texPath))
	img = DevIL.ilGenImage()
	DevIL.ilBindImage(img)
	if DevIL.ilLoadImage(bytestring(texPath)) != DevIL.IL_TRUE
		error("Failed loading texture $texPath")
	end

	w = DevIL.ilGetInteger(DevIL.IL_IMAGE_WIDTH)
	h = DevIL.ilGetInteger(DevIL.IL_IMAGE_HEIGHT)
	fmt = DevIL.ilGetInteger(DevIL.IL_IMAGE_FORMAT)
	typ = DevIL.ilGetInteger(DevIL.IL_IMAGE_TYPE)

	needConvert = typ != DevIL.IL_UNSIGNED_BYTE
	if !haskey(il2glFormat, fmt)
		fmt = DevIL.IL_RGBA
		needConvert = true
	end
	if needConvert
		if DevIL.ilConvertImage(fmt, DevIL.IL_UNSIGNED_BYTE) != DevIL.IL_TRUE
			error("Error converting texture $texPath to a supported format")
		end
	end
	pixelFormat = il2glFormat[fmt]

	data = DevIL.ilGetData()

	init(tex, renderer, data, w, h, pixelFormat; id = id)

	DevIL.ilDeleteImage(img)
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
