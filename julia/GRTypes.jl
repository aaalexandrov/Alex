immutable Vec2
	x::Float32
	y::Float32

	Vec2() = new()
	Vec2(x, y) = new(x, y)
end

immutable Vec3
	x::Float32
	y::Float32
	z::Float32

	Vec3() = new()
	Vec3(x, y, z) = new(x, y, z)
end

immutable Vec4
	x::Float32
	y::Float32
	z::Float32
	w::Float32

	Vec4() = new()
	Vec4(x, y, z, w) = new(x, y, z, w)
end

vec2array{T}(v::T) = [v.x, v.y, v.z]
position_func(positionField::Symbol) = v->vec2array(v.(positionField))

immutable MatrixColumn3
	e1::Float32
	e2::Float32
	e3::Float32
end

immutable MatrixColumn4
	e1::Float32
	e2::Float32
	e3::Float32
	e4::Float32
end

abstract AbstractImmutableMatrix

immutable Matrix3 <: AbstractImmutableMatrix
	c1::MatrixColumn3
	c2::MatrixColumn3
	c3::MatrixColumn3

	Matrix4() = new()
end

immutable Matrix4 <: AbstractImmutableMatrix
	c1::MatrixColumn4
	c2::MatrixColumn4
	c3::MatrixColumn4
	c4::MatrixColumn4

	Matrix4() = new()
end

import Base: size, eltype

eltype(::Type{Matrix3}) = Float32
size(::Type{Matrix3}) = (3, 3)

eltype(::Type{Matrix4}) = Float32
size(::Type{Matrix4}) = (4, 4)

size{T<:AbstractImmutableMatrix}(::Type{T}, i) = size(T)[i]

ismatrix(t::DataType) = t <: AbstractImmutableMatrix

type SamplerType{GLTYPE}
end

get_sampler_type{GLTYPE}(::Type{SamplerType{GLTYPE}}) = GLTYPE

const gl2jlTypes =
	Dict{UInt16, DataType}([
		(GL_FLOAT, Float32),
		(GL_FLOAT_VEC2, Vec2),
		(GL_FLOAT_VEC3, Vec3),
		(GL_FLOAT_VEC4, Vec4),
        (GL_FLOAT_MAT3, Matrix3),
		(GL_FLOAT_MAT4, Matrix4),
		(GL_SAMPLER_2D, SamplerType{Int64(GL_SAMPLER_2D)})
	])

gl2jltype(glType::Integer) = gl2jlTypes[glType]

const jl2glTypes =
	Dict{DataType, UInt16}([
		(Float32, GL_FLOAT),
		(Float64, GL_DOUBLE),
		(Float16, GL_HALF_FLOAT),
		(UInt16,  GL_UNSIGNED_SHORT),
		(Int16,   GL_SHORT),
		(UInt8,   GL_UNSIGNED_BYTE),
		(Int8,    GL_BYTE),
		(UInt32,  GL_UNSIGNED_INT),
		(Int32,   GL_INT)
	])

jl2gltype(jlType::DataType) = jl2glTypes[jlType]

function typeelements(dataType::DataType)
    fieldNames = fieldnames(dataType)
    if isempty(fieldNames)
        # simple type
        return (dataType, 1)
    else
        elType = Void
        for t in dataType.types
            if elType == Void
                elType = t
            elseif elType != t
                error("typeelements: A field with non-uniform types found")
            end
        end
        return (elType, length(fieldNames))
    end
end

function set_array_field{T}(vert::Vector{T}, field::Symbol, values::Array)
	fieldInd = findfirst(fieldnames(T), field)
	elType, elCount = typeelements(T.types[fieldInd])
	offs = fieldoffsets(T)[fieldInd]
	dst = convert(Ptr{elType}, pointer(vert)) + offs
	valueStride = div(length(values), length(vert))
	srcBase = 0
	for vertInd = 0:length(vert)-1
		for elInd = 1:elCount
			unsafe_store!(dst, values[srcBase+elInd], elInd)
		end
		dst += sizeof(T)
		srcBase += valueStride
	end
end

function set_array_fields{T}(vert::Vector{T}, fieldArrays::Dict{Symbol, Array})
	for (field, values) in fieldArrays
		set_array_field(vert, field, values)
	end
end
