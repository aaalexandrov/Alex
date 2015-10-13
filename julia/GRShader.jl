empty_setup_material(model) = nothing

type Shader <: Resource
	program::GLuint
	uniforms::Dict{Symbol, UniformVar}
	samplers::Vector{Symbol}
	blocks::Vector{UniformBlock}
    setupMaterial::Function
	id::Symbol
	attribType::DataType
	renderer::Renderer

	Shader() = new(0, Dict{Symbol, UniformVar}(), Symbol[], UniformBlock[])
end

isvalid(shader::Shader) = shader.program != 0

function init(shader::Shader, renderer::Renderer, path::AbstractString, setupMaterial::Function = empty_setup_material; id::Symbol = symbol(path))
	local vsSource, psSource
	open(path * ".vert") do f
		vsSource = readbytes(f)
	end

	open(path * ".frag") do f
		psSource = readbytes(f)
	end
	init(shader, renderer, pointer(vsSource), length(vsSource), pointer(psSource), length(psSource), setupMaterial, id = id)
end

function init(shader::Shader, renderer::Renderer, vs::Ptr{UInt8}, vsLength::Int, ps::Ptr{UInt8}, psLength::Int, setupMaterial::Function = empty_setup_material; id::Symbol = :shader)
	@assert !isvalid(shader)

	vertexShader = compileshader(GL_VERTEX_SHADER, vs, vsLength)
	if vertexShader != 0
		fragmentShader = compileshader(GL_FRAGMENT_SHADER, ps, psLength)
		if fragmentShader != 0
			shader.program = glCreateProgram()

			glAttachShader(shader.program, vertexShader)
			glAttachShader(shader.program, fragmentShader)

			glLinkProgram(shader.program)

			linkSuccess = GLint[0]
			glGetProgramiv(shader.program, GL_LINK_STATUS, linkSuccess)
			if linkSuccess[1] != GL_TRUE
                msg = get_program_info_log(shader.program)
				info("Error linking shader program id $id\n$msg")
				glDeleteProgram(shader.program)
				shader.program = 0
			end

			glDeleteShader(fragmentShader)
		end
		glDeleteShader(vertexShader)
	end

	if isvalid(shader)
        shader.setupMaterial = setupMaterial
		init_resource(shader, renderer, id)
		initblocks(shader)
		inituniforms(shader)

		initattributes(shader)
	end
end

function done(shader::Shader)
	if isvalid(shader)
		doneblocks(shader)
		glDeleteProgram(shader.program)
		shader.program = 0
		empty!(shader.uniforms)
		empty!(shader.samplers)
		remove_renderer_resource(shader)
	end
end

function apply(shader::Shader)
	@assert isvalid(shader)

	# possibly use glValidateProgram first?

	glUseProgram(shader.program)

	for block in shader.blocks
		apply(block)
	end
end

function get_info_log(glObj::GLuint, get::Function, getInfoLog::Function)
	logLength = GLint[0]
    get(glObj, GL_INFO_LOG_LENGTH, logLength)

    message = Array(UInt8, logLength[1])
    getInfoLog(glObj, logLength[1], C_NULL, message)
    bytestring(message)
end

get_shader_info_log(shaderObj::GLuint) = get_info_log(shaderObj, glGetShaderiv, glGetShaderInfoLog)
get_program_info_log(programObj::GLuint) = get_info_log(programObj, glGetProgramiv, glGetProgramInfoLog)

function compileshader(shaderType::UInt32, source::Ptr{UInt8}, sourceLength::Int)
	shaderObj = glCreateShader(shaderType)

	srcArray = Ptr{UInt8}[source]
	lenArray = GLint[sourceLength]
	glShaderSource(shaderObj, 1, srcArray, lenArray)

	glCompileShader(shaderObj)

	compileSuccess = GLint[0]
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, compileSuccess)

	if compileSuccess[1] != GL_TRUE
        msg = get_shader_info_log(shaderObj)
		shaderTypeName = shaderType == GL_VERTEX_SHADER ? "VERTEX" : (shaderType == GL_FRAGMENT_SHADER ? "FRAGMENT" : "$shaderType")
		info("Error compiling shader type $shaderTypeName\n$msg")

		glDeleteShader(shaderObj)
		shaderObj = 0
	end

	return shaderObj
end

function initblocks(shader::Shader)
	@assert isvalid(shader)
	@assert isempty(shader.blocks)

	blockCount = GLint[0]
	glGetProgramiv(shader.program, GL_ACTIVE_UNIFORM_BLOCKS, blockCount)
	@assert glGetError() == GL_NO_ERROR

	for i::GLuint = 0:blockCount[1]-1
		block = UniformBlock()
		init(block, shader.program, i)
		@assert isvalid(block)
		push!(shader.blocks, block)
	end
end

function get_block_id(shader::Shader, index::GLint)
	if index >= 0
		return findfirst(b -> b.index == index, shader.blocks)
	else
		return -1
	end
end

function inituniforms(shader::Shader)
	@assert isvalid(shader)
	@assert isempty(shader.uniforms)
	@assert isempty(shader.samplers)

	val = GLint[0]
	glGetProgramiv(shader.program, GL_ACTIVE_UNIFORMS, val)
	@assert glGetError() == GL_NO_ERROR

	uniformIndices = GLuint[i for i in 0:val[1]-1]
	count = length(uniformIndices)
	@assert count == val[1]

	uniformTypes = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, GL_UNIFORM_TYPE, uniformTypes)
	@assert glGetError() == GL_NO_ERROR

	uniformOffsets = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, GL_UNIFORM_OFFSET, uniformOffsets)
	@assert glGetError() == GL_NO_ERROR

	uniformArraySizes = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, GL_UNIFORM_SIZE, uniformArraySizes)
	@assert glGetError() == GL_NO_ERROR

	uniformArrayStrides = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, GL_UNIFORM_ARRAY_STRIDE, uniformArrayStrides)
	@assert glGetError() == GL_NO_ERROR

	uniformMatrixStrides = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, GL_UNIFORM_MATRIX_STRIDE, uniformMatrixStrides)
	@assert glGetError() == GL_NO_ERROR

	uniformBlockIndices = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, GL_UNIFORM_BLOCK_INDEX, uniformBlockIndices)
	@assert glGetError() == GL_NO_ERROR

	glGetProgramiv(shader.program, GL_ACTIVE_UNIFORM_MAX_LENGTH, val)
	@assert glGetError() == GL_NO_ERROR
	nameBuf = Array(UInt8, val[1])

	# set program so we can set sampler uniforms
	glUseProgram(shader.program)
	for i in 1:count
		glGetActiveUniformName(shader.program, uniformIndices[i], length(nameBuf), C_NULL, nameBuf)
		@assert glGetError() == GL_NO_ERROR

		var = UniformVar(symbol(bytestring(pointer(nameBuf))),
						 gl2jltype(uniformTypes[i]),
						 uniformIndices[i],
						 uniformOffsets[i],
						 uniformArraySizes[i],
						 uniformArrayStrides[i],
						 uniformMatrixStrides[i],
						 get_block_id(shader, uniformBlockIndices[i]))
		shader.uniforms[var.name] = var

		if var.varType <: SamplerType
			push!(shader.samplers, var.name)
			# set the sampler index once
			# sampler index can only be set via glUniform1i so we convert to Int32 to dispatch to that function
			glUniform(var.index, Int32(length(shader.samplers)))
		end
	end

	glUseProgram(0)
end

function doneblocks(shader::Shader)
	for block in shader.blocks
		done(block)
	end
	empty!(shader.blocks)
end

function initattributes(shader::Shader)
	@assert isvalid(shader)

	val = GLint[0]

	glGetProgramiv(shader.program, GL_ACTIVE_ATTRIBUTES, val)
	@assert glGetError() == GL_NO_ERROR
	attribCount = val[1]

	glGetProgramiv(shader.program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, val)
	@assert glGetError() == GL_NO_ERROR
	nameBuf = Array(UInt8, val[1])

	typeBuf = GLenum[0]
	sizeBuf = GLint[0]

	typeFields = Array(Expr, attribCount)
	for i = 1:attribCount
		glGetActiveAttrib(shader.program, i-1, length(nameBuf), C_NULL, sizeBuf, typeBuf, nameBuf)
		@assert glGetError() == GL_NO_ERROR
		
		location = glGetAttribLocation(shader.program, nameBuf) + 1
		@assert glGetError() == GL_NO_ERROR
		@assert 1 <= location <= attribCount

		# todo: add support for attribute arrays (via fixed size array types)
		if sizeBuf[1] != 1
			error("Vertex attribute arrays aren't supported in shader program $(shader.id), vertex attribute $nameBuf")
		end

		nameSym = symbol(bytestring(pointer(nameBuf)))
		jlType = gl2jltype(typeBuf[1])

		typeFields[location] = :($nameSym::$jlType)
	end

	typeSym = symbol("Vert#" * join(map(string, typeFields), "#"))
	typeExpr = quote
		immutable $typeSym
			$(typeFields...)
		end
		$typeSym
	end

	shader.attribType = eval(typeExpr)
end

get_sampler_index(shader::Shader, name::Symbol) = findfirst(shader.samplers, name)

function setuniform(shader::Shader, uniform::Symbol, value)
	@assert isvalid(shader)
	@assert gl_get_current_program() == shader.program

	if haskey(shader.uniforms, uniform)
		glUniform(shader.uniforms[uniform].index, value)
		return true
	end

	info("Attempt to set inexistent uniform $uniform in shader $(shader.name)")
    return false
end

function setuniform(shader::Shader, uniform::Symbol, tex::AbstractTexture)
	@assert isvalid(shader)
	@assert gl_get_current_program() == shader.program
	@assert isvalid(tex)

	texIndex = get_sampler_index(shader, uniform)
	if texIndex != 0
		@assert haskey(shader.uniforms, uniform)
		# apply the texture state to the texture unit we set on initialization
		apply(tex, texIndex)

		return true
	end

	return false
end
