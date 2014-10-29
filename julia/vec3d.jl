module Vec3D

import Base: size, getindex, setindex!, similar

export Vec, Vec3, Vec4, Mat4
export size, getindex, setindex!


abstract Vec{T} <: AbstractVector{T}
abstract Mat{T} <: AbstractMatrix{T}

type Vec3{T} <: Vec{T}
	x::T
	y::T
	z::T

	Vec3(x, y, z) = new(x, y, z)
	function Vec3()
		z = zero(T)
		new(z, z, z)
	end
end

type Vec4{T} <: Vec{T}
	x::T
	y::T
	z::T
	w::T

	Vec4(x, y, z, w) = new(x, y, z, w)
	function Vec4()
		z = zero(T)
		new(z, z, z, z)
	end
end

type Mat4{T} <: Mat{T}
	m11::T
	m21::T
	m31::T
	m41::T

	m12::T
	m22::T
	m32::T
	m42::T

	m13::T
	m23::T
	m33::T
	m43::T

	m14::T
	m24::T
	m34::T
	m44::T

	Mat4(m11, m21, m31, m41,
		 m12, m22, m32, m42,
		 m13, m23, m33, m43,
		 m14, m24, m34, m44) = new(m11, m21, m31, m41,
								   m12, m22, m32, m42,
								   m13, m23, m33, m43,
								   m14, m24, m34, m44)
	function Mat4()
		z = zero(T)
		o = one(T)
		new(o, z, z, z,
			z, o, z, z,
			z, z, o, z,
			z, z, z, o)
	end
end

size(::Vec3) = (3,)
size(::Vec4) = (4,)
size(::Mat4) = (4, 4)

function VecN{T}(::Type{T}, rows::Integer, cols::Integer = 1)
	if cols == 1
		if rows == 3
			return Vec3{T}()
		elseif rows == 4
			return Vec4{T}()
		end
	elseif cols == 4 && rows == 4
		return Mat4{T}()
	end
	error("invalid Vec dimensions")
end

similar{T, E}(v::Vec{T}, e::Type{E} = T, rows::Integer = size(v, 1), cols::Integer = 1) = VecN(E, rows, cols)
similar{T, E}(m::Mat{T}, e::Type{E} = T, rows::Integer = size(m, 1), cols::Integer = size(m, 2)) = VecN(E, rows, cols)
similar(v::Vec, e, t::Tuple) = similar(v, e, t...)
similar(m::Mat, e, t::Tuple) = similar(m, e, t...)

const vecFields = [:x, :y, :z, :w]
const matFields = [symbol("m$r$c") for r=1:4, c=1:4]

getindex(v::Vec, i::Integer) = getfield(v, vecFields[i])
setindex!(v::Vec, x, i::Integer) = setfield!(v, vecFields[i], x)

getindex(v::Mat, i::Integer, j::Integer) = getfield(v, matFields[i, j])
setindex!(v::Vec, x, i::Integer, j::Integer) = setfield!(v, matFields[i, j], x)



end
