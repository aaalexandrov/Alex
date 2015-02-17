module FixedVec

import Base: size, length, endof, ndims, getindex, setindex!, similar, start, next, done, dot, A_mul_B!

export size, length, endof, ndims, getindex, setindex!, similar, start, next, done, dot, A_mul_B!
export Vec, VecN


abstract Vec{T, SZ, N} <: AbstractArray{T, N}

function product(f, it)
	for i in it
		f(i)
	end
	nothing
end

function product(f, it1, it2)
	for i2 in it2, i1 in it1
		f(i1, i2)
	end
	nothing
end

function product(f, it1, it2, it3)
	for i3 in it3, i2 in it2, i1 in it1
		f(i1, i2, i3)
	end
	nothing
end

function product(f, it, its...)
	product(its...) do is...
		for i in it
			f(i, is...)
		end
	end
	nothing
end

tuple_to_string(t::(), sep) = ""
tuple_to_string(t::(Any,), sep) = "$(t[1])"
tuple_to_string(t::(Any, Any...), sep) = "$(t[1])$sep" * tuple_to_string(Base.tail(t), sep)

function field_names(sz::(Integer...); allowShort::Bool = true)
	const vecFields = [:x, :y, :z, :w]
	res = Array(Symbol, sz)
	n = 1
	shortNames = allowShort && sz[1] <= length(vecFields)
	product(shortNames? vecFields[1:sz[1]] : 1:sz[1], [1:sz[i] for i = 2:length(sz)]...) do is...
		field = tuple_to_string(tuple(is...), "_")
		if !shortNames
			field = "e" * field
		end
		res[n] = symbol(field)
		n += 1
	end
	return res
end

vec_name(sz::(Integer, Integer...)) = symbol("Vec" * tuple_to_string(sz, 'x'))

function gen_vec(sz::(Integer, Integer...))
	fieldNames = field_names(sz; allowShort = length(sz) == 1)
	fields = [:($(fieldNames[i])::T) for i = 1:length(fieldNames)]
	vecSym = vec_name(sz)
	constr = Expr(:(=), Expr(:call, vecSym, fieldNames...), Expr(:block, Expr(:call, :new, fieldNames...)))
	expo = Expr(:export, vecSym)
	res = quote
		type $vecSym{T} <: Vec{T, $sz, $(length(sz))}
			$(fields...)
			$vecSym() = new()
			$constr
		end
		$vecSym{T}(x::T, rest...) = $vecSym{T}(x, rest...)
		$expo
	end
	res
end

for n = 1:4
	eval(gen_vec((n,)))
end

for c=1:4, r=1:4
	eval(gen_vec((r, c)))
end

VecN{T}(::Type{T}, sz::(Integer, Integer...)) = subtypes(Vec{T, sz, length(sz)})[1]()
VecN(t::Type, dims::Integer...) = VecN(t, tuple(dims...))

size{T, SZ}(v::Vec{T, SZ}) = SZ
size{T, SZ, N}(v::Vec{T, SZ, N}, i::Int) = i <= N? SZ[i] : 1
length{T, SZ}(v::Vec{T, SZ}) = prod(SZ)::Int
endof{T, SZ}(v::Vec{T, SZ}) = prod(SZ)::Int
ndims{T, SZ, N}(v::Vec{T, SZ, N}) = N

similar{T, SZ}(v::Vec{T, SZ}, e = T, sz::(Integer, Integer...) = SZ) = VecN(e, sz)

start(v::Vec) = 1
done{T, SZ}(v::Vec{T, SZ}, state::Int) = state > prod(SZ)::Int
next{T, SZ}(v::Vec{T, SZ}, state::Int) = v[state], state+1

# needed to disambiguate with existing abstract array methods
function getindex{T}(v::Vec{T}, a::AbstractArray)
	res = VecN(T, length(a))
	n = 1
	for i in a
		res[n] = v[i]
		n += 1
	end
	return res
end

getindex(v::Vec, i::Real) = v.(i)
getindex{T, SZ}(v::Vec{T, SZ}, indices::Real...) = v.(sub2ind(SZ, indices...))
function getindex{T, SZ}(v::Vec{T, SZ}, indices...)
	lastDim = length(indices)
	while isa(indices[lastDim], Real)
		lastDim -= 1
	end
	szRes = tuple([length(indices[i]) for i in 1:lastDim]...)
	res = VecN(T, szRes)
	n = 1
	product(indices...) do is...
		res[n] = v[is...]
		n += 1
	end
	return res
end

setindex!(v::Vec, x, i::Real) = v.(i) = x
setindex!{T, SZ}(v::Vec{T, SZ}, x, indices::Real...) = v.(sub2ind(SZ, indices...)) = x
function setindex!{T, SZ}(v::Vec{T, SZ}, x, indices...)
	i = 1
	product(indices...) do is...
		v[is...] = x[i]
		i += 1
	end
	return x
end

dot{T}(u::Vec{T, (1,)}, v::Vec{T, (1,)}) = u.x*v.x
dot{T}(u::Vec{T, (2,)}, v::Vec{T, (2,)}) = u.x*v.x + u.y*v.y
dot{T}(u::Vec{T, (3,)}, v::Vec{T, (3,)}) = u.x*v.x + u.y*v.y + u.z*v.z
dot{T}(u::Vec{T, (4,)}, v::Vec{T, (4,)}) = u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w

function A_mul_B!{T}(dst::Vec{T, (4,)}, m::Vec{T, (4, 4)}, v::Vec{T, (4,)})
	dst.x = m.e1_1*v.x + m.e1_2*v.y + m.e1_3*v.z + m.e1_4*v.w
	dst.y = m.e2_1*v.x + m.e2_2*v.y + m.e2_3*v.z + m.e2_4*v.w
	dst.z = m.e3_1*v.x + m.e3_2*v.y + m.e3_3*v.z + m.e3_4*v.w
	dst.w = m.e4_1*v.x + m.e4_2*v.y + m.e4_3*v.z + m.e4_4*v.w
	dst
end

end
