module Vec3D

import Base: size, getindex, setindex!, similar

export Vec, Vec3, Vec4, Mat4
export size, getindex, setindex!


abstract Vec{T, N, SZ} <: AbstractArray{T, N}

const vecFields = [:x, :y, :z, :w]
const matFields = [symbol("m$r$c") for r=1:4, c=1:4]

function vec_fields(n)
	fields = [:($(vecFields[i])::T) for i = 1:n]
	fields
end

macro gen_vec(n)
	fields = vec_fields(n)
	vecSym = symbol("Vec$n")
	res = quote
		type $(vecSym){T} <: Vec{T, 1, ($n,)}
			$(fields...)
			$(vecSym)() = new()
		end
	end
	esc(res)
end

function mat_fields(rows, cols)
	fields = [:($(matFields[r, c])::T) for r = 1:rows, c = 1:cols]
	fields
end

macro gen_mat(r, c)
	fields = mat_fields(r, c)
	matSym = symbol("Mat$(r)x$(c)")
	res = quote
		type $(matSym){T} <: Vec{T, 2, ($r,$c)}
			$(fields...)
			$(matSym)() = new()
		end
	end
	esc(res)
end

for n = 1:4
	eval(macroexpand(:(@gen_vec $n)))
end

for c=1:4
	for r=1:4
		eval(macroexpand(:(@gen_mat $r $c)))
	end
end

VecN{T}(::Type{T}, sz::Tuple) = subtypes(Vec{T, length(sz), sz})[1]()
VecN(t::Type, dims::Integer...) = VecN(t, tuple(dims...))

size{T, N, SZ}(v::Vec{T, N, SZ}) = SZ

similar{T, N, SZ}(v::Vec{T, N, SZ}, e = T, t::Tuple = SZ) = VecN(e, t)

getindex(v::Vec, i::Integer) = getfield(v, vecFields[i])
setindex!{T, N, SZ}(v::Vec{T, N, SZ}, x, i::Integer) = setfield!(v, vecFields[i], convert(T, x))

getindex{T, SZ}(v::Vec{T, 2, SZ}, i::Integer, j::Integer) = getfield(v, matFields[i, j])
setindex!{T, SZ}(v::Vec{T, 2, SZ}, x, i::Integer, j::Integer) = setfield!(v, matFields[i, j], convert(T, x))

function dot{T, R}(u::Vec{T, 1, (R,)}, v::Vec{T, 1, (R,)})
	res = u.x*v.x
	for i=2:R
		res += u[i]*v[i]
	end
	res
end

function dot{T}(u::Vec{T, 1, (1,)}, v::Vec{T, 1, (1,)})
	return u.x*v.x
end

function dot{T}(u::Vec{T, 1, (2,)}, v::Vec{T, 1, (2,)})
	return u.x*v.x + u.y*v.y
end

function dot{T}(u::Vec{T, 1, (3,)}, v::Vec{T, 1, (3,)})
	return u.x*v.x + u.y*v.y + u.z*v.z
end

function dot{T}(u::Vec{T, 1, (4,)}, v::Vec{T, 1, (4,)})
	return u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w
end


end
