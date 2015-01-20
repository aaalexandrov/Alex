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
	Dict{Uint16, DataType}([
		(GL_FLOAT, Float32),
		(GL_FLOAT_VEC2, Vec2),
		(GL_FLOAT_VEC3, Vec3),
		(GL_FLOAT_VEC4, Vec4),
		(GL_FLOAT_MAT4, Matrix4),
		(GL_SAMPLER_2D, SamplerType{int64(GL_SAMPLER_2D)})
	])

gl2jltype(glType::Integer) = gl2jlTypes[glType]

const jl2glTypes =
	Dict{DataType, Uint16}([
		(Float32, GL_FLOAT),
		(Float64, GL_DOUBLE),
		(Float16, GL_HALF_FLOAT),
		(Uint16,  GL_UNSIGNED_SHORT),
		(Int16,   GL_SHORT),
		(Uint8,   GL_UNSIGNED_BYTE),
		(Int8,    GL_BYTE),
		(Uint32,  GL_UNSIGNED_INT),
		(Int32,   GL_INT)
	])

jl2gltype(jlType::DataType) = jl2glTypes[jlType]
