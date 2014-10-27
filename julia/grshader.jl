type Shader <: Resource
	program::GLuint
	uniforms::Dict{Symbol, UniformVar}
	samplers::Vector{Symbol}
	blocks::Vector{UniformBlock}
	worldTransform::Symbol
	
	Shader() = new(0, Dict{Symbol, UniformVar}(), Symbol[], UniformBlock[], :model)
end

isvalid(shader::Shader) = shader.program != 0

function init(shader::Shader, vsPath::String, psPath::String)
	local vsSource, psSource
	open(vsPath) do f
		vsSource = readbytes(f)
	end
	
	open(psPath) do f
		psSource = readbytes(f)
	end
	
	init(shader, pointer(vsSource), length(vsSource), pointer(psSource), length(psSource))
end

function init(shader::Shader, vs::Ptr{Uint8}, vsLength::Int, ps::Ptr{Uint8}, psLength::Int)
	@assert !isvalid(shader)
	
	vertexShader = compileshader(VERTEX_SHADER, vs, vsLength)
	if vertexShader != 0
		fragmentShader = compileshader(FRAGMENT_SHADER, ps, psLength)
		if fragmentShader != 0
			shader.program = glCreateProgram()
			
			glAttachShader(shader.program, vertexShader)
			glAttachShader(shader.program, fragmentShader)
			
			glLinkProgram(shader.program)
			
			linkSuccess = GLint[0]
			glGetProgramiv(shader.program, LINK_STATUS, linkSuccess)
			if linkSuccess[1] != TRUE
				glDeleteProgram(shader.program)
				shader.program = 0
				info("Error linking shader program")
			end
			
			glDeleteShader(fragmentShader)
		end
		glDeleteShader(vertexShader)
	end
	
	if isvalid(shader)
		initblocks(shader)
		inituniforms(shader)
	end
end

function done(shader::Shader)
	if isvalid(shader)
		doneblocks(shader)
		glDeleteProgram(shader.program)
		shader.program = 0
		empty!(shader.uniforms)
		empty!(shader.samplers)
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

function compileshader(shaderType::Uint16, source::Ptr{Uint8}, sourceLength::Int)
	shaderObj = glCreateShader(shaderType)
	
	srcArray = Ptr{Uint8}[source]
	lenArray = GLint[sourceLength]
	glShaderSource(shaderObj, 1, srcArray, lenArray)
	
	glCompileShader(shaderObj)
	
	compileSuccess = GLint[0]
	glGetShaderiv(shaderObj, COMPILE_STATUS, compileSuccess)
	
	if compileSuccess[1] != TRUE
		logLength = GLint[0]
		glGetShaderiv(shaderObj, INFO_LOG_LENGTH, logLength)
		
		message = Array(Uint8, logLength[1])
		glGetShaderInfoLog(shaderObj, logLength[1], C_NULL, message)
		msg = bytestring(message)
		shaderTypeName = shaderType == VERTEX_SHADER ? "VERTEX" : (shaderType == FRAGMENT_SHADER ? "FRAGMENT" : "$shaderType")
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
	glGetProgramiv(shader.program, ACTIVE_UNIFORM_BLOCKS, blockCount)
	@assert glGetError() == NO_ERROR
	
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
	glGetProgramiv(shader.program, ACTIVE_UNIFORMS, val)
	@assert glGetError() == NO_ERROR

	uniformIndices = GLuint[i for i in 0:val[1]-1]
	count = length(uniformIndices)
	@assert count == val[1]
	
	uniformTypes = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, UNIFORM_TYPE, uniformTypes)
	@assert glGetError() == NO_ERROR

	uniformOffsets = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, UNIFORM_OFFSET, uniformOffsets)
	@assert glGetError() == NO_ERROR
	
	uniformArraySizes = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, UNIFORM_SIZE, uniformArraySizes)
	@assert glGetError() == NO_ERROR

	uniformArrayStrides = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, UNIFORM_ARRAY_STRIDE, uniformArrayStrides)
	@assert glGetError() == NO_ERROR

	uniformMatrixStrides = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, UNIFORM_MATRIX_STRIDE, uniformMatrixStrides)
	@assert glGetError() == NO_ERROR
	
	uniformBlockIndices = Array(GLint, count)
	glGetActiveUniformsiv(shader.program, count, uniformIndices, UNIFORM_BLOCK_INDEX, uniformBlockIndices)
	@assert glGetError() == NO_ERROR
	
	glGetProgramiv(shader.program, ACTIVE_UNIFORM_MAX_LENGTH, val)
	@assert glGetError() == NO_ERROR
	nameBuf = Array(Uint8, val[1])

	# set program so we can set sampler uniforms
	glUseProgram(shader.program)
	
	for i in 1:count
		glGetActiveUniformName(shader.program, uniformIndices[i], length(nameBuf), C_NULL, nameBuf)
		@assert glGetError() == NO_ERROR
		
		var = UniformVar(symbol(bytestring(nameBuf)),
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
			glUniform(var.index, int32(length(shader.samplers)))
		end
	end
	
	@assert haskey(shader.uniforms, shader.worldTransform)

	glUseProgram(0)
end

function doneblocks(shader::Shader)
	for block in shader.blocks
		done(block)
	end
	empty!(shader.blocks)
end

get_sampler_index(shader::Shader, name::Symbol) = findfirst(shader.samplers, name)

function setuniform(shader::Shader, uniform::Symbol, value)
	@assert isvalid(shader)
	@assert gl_get_current_program() == shader.program
	
	if haskey(shader.uniforms, uniform)
		glUniform(shader.uniforms[uniform].index, value)
		return true
	end
	
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

