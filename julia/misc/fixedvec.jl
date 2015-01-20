module FixedVec

import Base: size, getindex, setindex!, similar, dot, A_mul_B!

export size, getindex, setindex!, similar, dot, A_mul_B!
export Vec, VecN


abstract Vec{T, SZ, N} <: AbstractArray{T, N}

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

tuple_to_string(t::(), sep) = ""
tuple_to_string(t::(Any,), sep) = "$(t[1])"
tuple_to_string(t::(Any, Any...), sep) = "$(t[1])$sep" * tuple_to_string(Base.tail(t), sep)

vec_name(sz::(Integer, Integer...)) = symbol("Vec" * tuple_to_string(sz, 'x'))

function gen_vec(sz::(Integer, Integer...))
	fieldNames = field_names(sz)
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
        field_names{T}(::Vec{T, $sz, $(length(sz))}) = $fieldNames
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

similar{T, SZ}(v::Vec{T, SZ}, e = T, sz::(Integer, Integer...) = SZ) = VecN(e, sz)

# these are needed to disambiguate with existing abstract array methods
getindex(v::Vec, i::Real) = getfield(v, field_names(v)[i])
getindex(v::Vec, ind::AbstractArray) = map(f->getfield(v, f), field_names(v)[ind])
setindex!(v::Vec, x, i::Real) = setfield!(v, field_names(v)[i], x)

# general case
getindex(v::Vec, indices::Real...) = getfield(v, field_names(v)[indices...])
getindex(v::Vec, indices...) = map(f->getfield(v, f), field_names(v)[indices...]) # should we return a fixed size array instead of normal one here?

setindex!(v::Vec, x, indices::Real...) = setfield!(v, field_names(v)[indices...], x)
function setindex!(v::Vec, x, indices...)
    fieldArr = field_names(v)[indices...]
    for i = 1:length(fieldArr)
        setfield!(v, fieldArr[i], x[i])
    end
    v
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
