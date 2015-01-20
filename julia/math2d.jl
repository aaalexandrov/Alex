module Math2D

import Base: size, eltype, isempty, min, max, union, intersect, zero, one, dot, typemin, typemax

export Vec2, Box, Rect
export size, eltype, len, len2, min, max, isempty, union, intersect, zero, one, normalize, orthogonal, dot, rect, typemin, typemax

immutable Vec2{T}
    x::T
    y::T
end

size{T}(::Type{Vec2{T}}) = (2,)
size(v::Vec2) = size(typeof(v))
eltype{T}(::Type{Vec2{T}}) = T
eltype(v::Vec2) = eltype(typeof(v))

dot(v1::Vec2, v2::Vec2) = v1.x*v2.x + v1.y*v2.y

len2(v::Vec2) = v.x*v.x + v.y*v.y
len(v::Vec2) = sqrt(len2(v))

function normalize{T}(v::Vec2{T})
    l = len(v)
    l < eps(T) ? v : v / l
end

orthogonal(v::Vec2) = Vec2(v.y, -v.x)

zero{T}(::Type{Vec2{T}}) = Vec2(zero(T), zero(T))
zero(v::Vec2) = zero(typeof(v))
one{T}(::Type{Vec2{T}}) = Vec2(one(T), one(T))
one(v::Vec2) = one(typeof(v))
typemin{T}(::Type{Vec2{T}}) = Vec2(typemin(T), typemin(T))
typemin(v::Vec2) = typemin(typeof(v))
typemax{T}(::Type{Vec2{T}}) = Vec2(typemax(T), typemax(T))
typemax(v::Vec2) = typemax(typeof(v))

(+)(v::Vec2) = v
(-)(v::Vec2) = Vec2(-v.x, -v.y)

(+)(v1::Vec2, v2::Vec2) = Vec2(v1.x + v2.x, v1.y + v2.y)
(-)(v1::Vec2, v2::Vec2) = Vec2(v1.x - v2.x, v1.y - v2.y)
(*)(v1::Vec2, v2::Vec2) = Vec2(v1.x * v2.x, v1.y * v2.y)
(/)(v1::Vec2, v2::Vec2) = Vec2(v1.x / v2.x, v1.y / v2.y)

(*)(v::Vec2, n::Number) = Vec2(v.x * n, v.y * n)
(*)(n::Number, v::Vec2) = Vec2(n * v.x, n * v.y)
(/)(v::Vec2, n::Number) = Vec2(v.x / n, v.y / n)
(/)(n::Number, v::Vec2) = Vec2(n / v.x, n / v.y)

(.<)(v1::Vec2, v2::Vec2) = v1.x < v2.x && v1.y < v2.y

min(v::Vec2) = v
min(v1::Vec2, v2::Vec2) = Vec2(min(v1.x, v2.x), min(v1.y, v2.y))
min(v::Vec2, vs::Vec2...) = min(v, min(vs...))

max(v::Vec2) = v
max(v1::Vec2, v2::Vec2) = Vec2(max(v1.x, v2.x), max(v1.y, v2.y))
max(v::Vec2, vs::Vec2...) = max(v, max(vs...))

type Box{V}
    min::V
    max::V
end

size(b::Box) = b.max - b.min
isempty(b::Box) = b.max .< b.min

union(bs::Box...) = Box(min([b.min for b in bs]), max([b.max for b in bs]))
intersect(bs::Box...) = Box(max([b.min for b in bs]), min([b.max for b in bs]))

typealias Rect{T} Box{Vec2{T}}

rect(T, minX, minY, maxX, maxY) = Box(Vec2{T}(minX, minY), Vec2{T}(maxX, maxY))
rect(T) = Box(typemax(Vec2{T}), typemin(Vec2{T}))

end
