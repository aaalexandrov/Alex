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

immutable MatrixColumn4
	e1::Float32
	e2::Float32
	e3::Float32
	e4::Float32
end

abstract AbstractImmutableMatrix

immutable Matrix4 <: AbstractImmutableMatrix
	c1::MatrixColumn4
	c2::MatrixColumn4
	c3::MatrixColumn4
	c4::MatrixColumn4
	
	Matrix4() = new()
end

import Base: size, eltype

eltype(::Type{Matrix4}) = Float32
size(::Type{Matrix4}) = (4, 4)

ismatrix(t::DataType) = t <: AbstractImmutableMatrix

type SamplerType{GLTYPE}
end

get_sampler_type{GLTYPE}(::Type{SamplerType{GLTYPE}}) = GLTYPE

const gl2jlTypes = 
	(Uint16 => DataType)[FLOAT => Float32, FLOAT_VEC2 => Vec2, FLOAT_VEC3 => Vec3, FLOAT_VEC4 => Vec4, FLOAT_MAT4 => Matrix4, SAMPLER_2D => SamplerType{int64(SAMPLER_2D)}]

gl2jltype(glType::Integer) = gl2jlTypes[glType]

const jl2glTypes = 
	(DataType => Uint16)[Float32 => FLOAT, Float64 => DOUBLE, Float16 => HALF_FLOAT, Uint16 => UNSIGNED_SHORT, Int16 => SHORT, Uint8 => UNSIGNED_BYTE, Int8 => BYTE, Uint32 => UNSIGNED_INT, Int32 => INT]

jl2gltype(jlType::DataType) = jl2glTypes[jlType]
