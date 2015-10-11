type UniformBlock
	ubo::GLuint
	index::GLuint
	binding::GLuint
	size::Uint32
	name::Symbol

	UniformBlock() = new (0, 0, 0, 0)
end

isvalid(block::UniformBlock) = block.ubo != 0

function init(block::UniformBlock, program::GLuint, blockIndex::GLuint)
	block.index = blockIndex
	val = GLint[1]

    block.binding = block.index
    glUniformBlockBinding(program, block.index, block.binding)
    
	glGetActiveUniformBlockiv(program, block.index, GL_UNIFORM_BLOCK_DATA_SIZE, val)
	@assert glGetError() == GL_NO_ERROR
	block.size = val[1]

	glGetActiveUniformBlockiv(program, block.index, GL_UNIFORM_BLOCK_NAME_LENGTH, val)
	@assert glGetError() == GL_NO_ERROR
	nameBuf = Array(Uint8, val[1])
	glGetActiveUniformBlockName(program, block.index, length(nameBuf), C_NULL, nameBuf)
	@assert glGetError() == GL_NO_ERROR
	block.name = symbol(nameBuf[1:end-1])

	ubo = GLuint[0]
	glGenBuffers(1, ubo)
	block.ubo = ubo[1]

	data = zeros(Uint8, block.size)
	setbufferdata(block, data)
end

function done(block::UniformBlock)
	if isvalid(block)
		ubo = GLuint[block.ubo]
		glDeleteBuffers(1, ubo)
		block.ubo = 0
	end
end

function setbufferdata(block::UniformBlock, data)
	@assert sizeof(data) == block.size
	glBindBuffer(GL_UNIFORM_BUFFER, block.ubo)
	glBufferData(GL_UNIFORM_BUFFER, sizeof(data), data, GL_DYNAMIC_DRAW)
end

function apply(block::UniformBlock)
	glBindBufferBase(GL_UNIFORM_BUFFER, block.binding, block.ubo)
end


type UniformVar
	name::Symbol
	varType::DataType
	index::GLint
	offset::Int32
	arraySize::Int32
	arrayStride::Int32
	matrixStride::Int32
	blockId::Int32
end

inblock(var::UniformVar) = var.blockId >= 0

function getptr(var::UniformVar, buffer::Vector{Uint8}, index::Int)
	@assert 1 <= index <= var.arraySize
	ptr = pointer(buffer) + var.offset + var.arrayStride * (index - 1)
	@assert convert(Uint64, pointer(buffer)) <= convert(Uint64, ptr + sizeof(var.varType)) <= convert(Uint64, pointer(buffer) + length(buffer))
	return convert(Ptr{var.varType}, ptr)
end

function setvalue(var::UniformVar, buffer::Vector{Uint8}, value, index::Int = 1)
	ptr = getptr(var, buffer, index)
	unsafe_store!(ptr, value)
end

function setvalue(var::UniformVar, buffer::Vector{Uint8}, array::Array, index::Int = 1)
	ptr = getptr(var, buffer, index)
	# @assert ismatrix(var.varType)
    if sizeof(var.varType) != sizeof(array)
        info("$(var.name) has size $(sizeof(var.varType)), trying to set value of size $(sizeof(array))")
        @assert sizeof(var.varType) == sizeof(array)
    end

    if !ismatrix(var.varType) || size(var.varType, 1) == 4
        unsafe_copy!(convert(Ptr{Uint8}, ptr), convert(Ptr{Uint8}, pointer(array)), sizeof(var.varType))
    else
        elType = eltype(var.varType)
        elSize = sizeof(elType)
        rowSize = size(var.varType, 1)
        @assert rowSize < 4
        pSrc = convert(Ptr{elType}, pointer(array))
        pDst = convert(Ptr{elType}, ptr)
        for col = 1:length(array)/rowSize
            unsafe_copy!(pDst, pSrc, rowSize)
            pDst += 4elSize
            pSrc += rowSize * elSize
        end
        @assert pSrc == pointer(array) + length(array) * sizeof(eltype(array))
    end
end

load_ptr{T}(p::Ptr{T}) = unsafe_load(p)

function getvalue(var::UniformVar, buffer::Vector{Uint8}, index::Int = 1)
	ptr = getptr(var, buffer, index)

	if ismatrix(var.varType)
		result = Array(eltype(var.varType), size(var.varType))
		@assert sizeof(result) == sizeof(var.varType)
		unsafe_copy!(convert(Ptr{Uint8}, pointer(result)), convert(Ptr{Uint8}, ptr), sizeof(var.varType))
		return result
	else
		# for some reason unsafe_load can't be called directly here, wrap it in a function
		# return unsafe_load(ptr)
		return load_ptr(ptr)
	end
end
