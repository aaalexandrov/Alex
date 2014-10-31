module FixedVec

import Base: size, getindex, setindex!, similar, dot

export size, getindex, setindex!, similar, dot
export Vec, VecN


abstract Vec{T, SZ, N} <: AbstractArray{T, N}

const vecFields1d = [:x, :y, :z, :w]
const vecFields2d = [symbol("m$(r)_$c") for r=1:4, c=1:4]

function field_names(sz::(Integer,); allowShort::Bool = true)
	n = sz[1]
	const vecFields = [:x, :y, :z, :w]
	if allowShort && n <= length(vecFields)
		return vecFields[1:n]
	end
	[symbol("e$i") for i=1:n]
end

function field_names(sz::(Integer, Integer...); allowShort::Bool = false)
	firstNames = field_names(sz[1:end-1], allowShort = false)
	names = Array(Symbol, sz)
	ind = 1
	for i = 1:sz[end]
		for j = 1:length(firstNames)
			pref = string(firstNames[j])
			names[ind] = symbol("$(pref)_$i")
			ind += 1
		end
	end
	names
end

vec_fields(sz::(Integer,)) = [:($(vecFields1d[i])::T) for i = 1:sz[1]]
vec_fields(sz::(Integer, Integer)) = [:($(vecFields2d[r, c])::T) for r = 1:sz[1], c = 1:sz[2]]

tuple_to_string(t::(), sep) = ""
tuple_to_string(t::(Any,), sep) = "$(t[1])"
tuple_to_string(t::(Any, Any...), sep) = "$(t[1])$sep" * tuple_to_string(Base.tail(t), sep)

vec_name(sz::(Integer, Integer...)) = symbol("Vec" * tuple_to_string(sz, 'x'))

function gen_vec(sz::(Integer, Integer...))
	fields = vec_fields(sz)
	params = [p.args[1] for p in fields]
	vecSym = vec_name(sz)
	constr = Expr(:(=), Expr(:call, vecSym, params...), Expr(:block, Expr(:call, :new, params...)))
	exp = Expr(:export, vecSym)
	res = quote
		type $vecSym{T} <: Vec{T, $sz, $(length(sz))}
			$(fields...)
			$vecSym() = new()
			$constr
		end
		$vecSym{T}(x::T, rest...) = $vecSym{T}(x, rest...)
		$exp
	end
	res
end

for n = 1:4
	eval(gen_vec((n,)))
end

for c=1:4
	for r=1:4
		eval(gen_vec((r, c)))
	end
end

VecN{T}(::Type{T}, sz::(Integer, Integer...)) = subtypes(Vec{T, sz, length(sz)})[1]()
VecN(t::Type, dims::Integer...) = VecN(t, tuple(dims...))

size{T, SZ}(v::Vec{T, SZ}) = SZ

similar{T, SZ}(v::Vec{T, SZ}, e = T, sz::(Integer, Integer...) = SZ) = VecN(e, sz)

getindex{T, R}(v::Vec{T, (R,)}, i::Integer) = getfield(v, vecFields1d[i])
setindex!{T, R}(v::Vec{T, (R,)}, x, i::Integer) = setfield!(v, vecFields1d[i], convert(T, x))

function ind1to2d(rows::Integer, i::Integer)
	c, r = divrem(i-1, rows)
	r + 1, c + 1
end

getindex{T, R, C}(v::Vec{T, (R, C)}, i::Integer) = getfield(v, vecFields2d[ind1to2d(R, i)...])
setindex!{T, R, C}(v::Vec{T, (R, C)}, x, i::Integer) = setfield!(v, vecFields2d[ind1to2d(R, i)...], convert(T, x))

getindex{T, R, C}(v::Vec{T, (R, C)}, i::Integer, j::Integer) = getfield(v, vecFields2d[i, j])
setindex!{T, R, C}(v::Vec{T, (R, C)}, x, i::Integer, j::Integer) = setfield!(v, vecFields2d[i, j], convert(T, x))

dot{T}(u::Vec{T, (1,)}, v::Vec{T, (1,)}) = u.x*v.x
dot{T}(u::Vec{T, (2,)}, v::Vec{T, (2,)}) = u.x*v.x + u.y*v.y
dot{T}(u::Vec{T, (3,)}, v::Vec{T, (3,)}) = u.x*v.x + u.y*v.y + u.z*v.z
dot{T}(u::Vec{T, (4,)}, v::Vec{T, (4,)}) = u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w


end
