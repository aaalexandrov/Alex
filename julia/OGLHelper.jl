module OGLHelper

using ModernGL

glUniform(index::GLint, value::Float32) = glUniform1f(index, value)
glUniform(index::GLint, value::(Float32, Float32)) = glUniform2f(index, value[1], value[2])
glUniform(index::GLint, value::(Float32, Float32, Float32)) = glUniform3f(index, value[1], value[2], value[3])
glUniform(index::GLint, value::(Float32, Float32, Float32, Float32)) = glUniform4f(index, value[1], value[2], value[3], value[4])

glUniform(index::GLint, value::Int32) = glUniform1i(index, value)
glUniform(index::GLint, value::(Int32, Int32)) = glUniform2i(index, value[1], value[2])
glUniform(index::GLint, value::(Int32, Int32, Int32)) = glUniform3i(index, value[1], value[2], value[3])
glUniform(index::GLint, value::(Int32, Int32, Int32, Int32)) = glUniform4i(index, value[1], value[2], value[3], value[4])

glUniform(index::GLint, value::Uint32) = glUniform1ui(index, value)
glUniform(index::GLint, value::(Uint32, Uint32)) = glUniform2ui(index, value[1], value[2])
glUniform(index::GLint, value::(Uint32, Uint32, Uint32)) = glUniform3ui(index, value[1], value[2], value[3])
glUniform(index::GLint, value::(Uint32, Uint32, Uint32, Uint32)) = glUniform4ui(index, value[1], value[2], value[3], value[4])

nofunc(x...) = nothing

macro notranspose(func)
	quote
		function (location::GLint, count::GLsizei, value::Ptr{GLfloat})
			$func(location, count, GL_FALSE, value)
		end
	end
end

const uniformFuncTable =
	[glUniform1fv         nofunc                             nofunc                             nofunc                            ;
	 glUniform2fv         @notranspose(glUniformMatrix2fv)   @notranspose(glUniformMatrix3x2fv) @notranspose(glUniformMatrix4x2fv);
	 glUniform3fv         @notranspose(glUniformMatrix2x3fv) @notranspose(glUniformMatrix3fv)   @notranspose(glUniformMatrix4x3fv);
	 glUniform4fv         @notranspose(glUniformMatrix2x4fv) @notranspose(glUniformMatrix3x4fv) @notranspose(glUniformMatrix4fv)  ]

function get_uniform_func_float(rows::Int, cols::Int)
	func = uniformFuncTable[rows, cols]
	@assert func != nofunc
	return func
end

function glUniform(index::GLint, value::Vector{Float32})
	glUniformFunc = get_uniform_func_float(length(value), 1)
	glUniformFunc(index, one(GLsizei), convert(Ptr{Float32}, value))
end

function glUniform(index::GLint, value::Matrix{Float32})
	rows, cols = size(value)
	glUniformMatrixFunc = get_uniform_func_float(rows, cols)
	glUniformMatrixFunc(index, one(GLsizei), convert(Ptr{Float32}, value))
end

function glUniform(index::GLint, value::Array{Float32, 3})
	rows, cols, elements = size(value)
	local glUniformFunc
	glUniformFunc = get_uniform_func_float(rows, cols)
	glUniformFunc(index, convert(GLsizei, elements), convert(Ptr{Float32}, value))
end

# todo: add glUniform for Vector{Int32 & Uint32}

function gl_get_current_program()
	currentProgram = GLint[0]
	glGetIntegerv(GL_CURRENT_PROGRAM, currentProgram)
	return currentProgram[1]
end

function gl_clear_buffers(color, depth, stencil)
	mask = 0
	if color != nothing
		glClearColor(color...)
		mask |= GL_COLOR_BUFFER_BIT
	end
	if depth != nothing
		glClearDepth(depth)
		mask |= GL_DEPTH_BUFFER_BIT
	end
	if stencil != nothing
		glClearStencil(stencil)
		mask |= GL_STENCIL_BUFFER_BIT
	end
	glClear(mask)
end

function gl_info()
	vendor = bytestring(glGetString(GL_VENDOR))
	version = bytestring(glGetString(GL_VERSION))
	renderer = bytestring(glGetString(GL_RENDERER))
	info("OpenGL Vendor: $vendor, Version: $version, Renderer: $renderer")
end


export glUniform, gl_get_current_program, gl_clear_buffers, gl_info

end
