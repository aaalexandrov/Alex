module Shapes

export Shape, Line, Plane, Sphere, AABB, Convex
export isvalid, getnormal, getpoint, volume, addpoint, setplane, getintersection, intersect, outside


iszero{T}(x::T) = abs(x) < eps(T)

len2(x) = sumabs2(x)
len(x) = sqrt(len2(x))

lerp(x, y, t) = x + (y-x)*t

make_interval(a, b) = a < b ? (a, b) : (b, a)
empty_interval(i) = i[1] > i[2]
intersect_interval(min1, max1, min2, max2) = (max(min1, min2), min(max1, max2))

# roots of ax^2+bx+c=0
function quadroots(a, b, c)
	d = b*b-4a*c
	if d < zero(d)
		return (nan(a), nan(a))
	end
	sd = sqrt(d)
	x1 = (-b-sd)/2a
	x2 = (-b+sd)/2a
	return x1, x2
end


abstract Shape{T <: Real}


type Line{T} <: Shape{T}
	p::Array{T, 2}
end

Line{T}(x1::T, y1, z1, x2, y2, z2) = Line{T}(T[x1 x2; y1 y2; z1 z2])
Line(p0, p1) = Line(p0..., p1...)

isvalid{T}(l::Line{T}) = size(l.p)==(3, 2) && len2(l.p[:, 1] - l.p[:, 2]) >= eps(T)

getvector{T}(l::Line{T}) = l.p[:, 2] - l.p[:, 1]
getpoint{T}(l::Line{T}, t::T) = lerp(l.p[:, 1], l.p[:, 2], t)


type Plane{T} <: Shape{T}
	p::Array{T, 1}
end

Plane{T}(a::T, b, c, d) = Plane{T}(T[a, b, c, d])
Plane(n, p) = Plane(n..., -dot(p, n))

isvalid{T}(p::Plane{T}) = size(p.p) == (4,) && len2(p.p[1:3]) >= eps(T)

getnormal{T}(p::Plane{T}) = p.p[1:3]
getvalue{T}(p::Plane{T}, point::Vector{T}) = dot(p.p[1:3], point[1:3]) + p.p[4]

function getpoint{T}(p::Plane{T})
	i = indmax(p.p[1:3])
	return T[x==i ? -p.p[4]/p.p[i] : 0 for x=1:3]
end


type Sphere{T} <: Shape{T}
	c::Array{T, 1}
	r::T
end

Sphere{T}(cx::T, cy, cz, r) = Sphere{T}(T[cx, cy, cz], convert(T, r))
Sphere(c, z) = Sphere(c..., z)

isvalid{T}(s::Sphere{T}) = size(s.c) == (3,) && s.r >= zero(T)
volume(s::Sphere) = s.r^3*4pi/3

function addpoint{T}(s::Sphere{T}, p::Vector{T})
	@assert isvalid(s)
	@assert length(p) == 3

	d = len(p-s.c)
	if d > s.r
		s.r = (d+s.r)/2
		s.c = lerp(p, s.c, s.r/d)
	end
	s
end

type AABB{T} <: Shape{T}
	p::Array{T, 2}
end

AABB{T}(xmin::T, ymin, zmin, xmax, ymax, zmax) = AABB{T}(T[xmin xmax; ymin ymax; zmin zmax])
AABB(pmin, pmax) = AABB(pmin..., pmax...)

isvalid{T}(aabb::AABB{T}) = size(aabb.p) == (3, 2) && aabb.p[1, 1] <= aabb.p[1, 2] && aabb.p[2, 1] <= aabb.p[2, 2] && aabb.p[3, 1] <= aabb.p[3, 2]
volume(ab::AABB) = prod([ab.p[i, 2] - ab.p[i, 1] for i=1:3])

function addpoint{T}(ab::AABB{T}, p::Vector{T})
	@assert isvalid(ab)
	@assert length(p) == 3

	for i = 1:3
		if p[i] < ab.p[i, 1]
			ab.p[i, 1] = p[i]
		end
		if ab.p[i, 2] < p[i]
			ab.p[i, 2] = p[i]
		end
	end
	ab
end

# todo: add Capsule and OBB


type Convex{T} <: Shape{T}
	planes::Array{T, 2}
end
# todo: add edges and vertices and intersection functions

Convex{T}(::Type{T}, planeCount::Int) = Convex{T}(zeros(T, 4, planeCount))

function Convex{T}(planes::Plane{T}...)
	c = Convex(T, length(planes))
	for i in 1:length(planes)
		setplane(c, i, planes[i])
	end
	c
end

isvalid{T}(c::Convex{T}) = size(planes, 1) == 4 && size(planes, 2) > 0
setplane{T}(c::Convex{T}, planeIndex::Int, p::Vector{T}) = c.planes[:, planeIndex] = p / len(p[1:3])


function getintersection{T}(l::Line{T}, p::Plane{T})
	@assert isvalid(l)
	@assert isvalid(p)

	lo = l.p[:, 1]
	lv = l.p[:, 2] - lo
	pn = getnormal(p)
	po = getpoint(p)

	vn = dot(lv, pn)
	d = dot(po-lo, pn)
	if iszero(vn)
		# line and plane are parallel
		return iszero(d) ? inf(T) : nan(T)
	end

	return d / vn
end

intersect{T}(l::Line{T}, p::Plane{T}) = getintersection(l, p) != nan(T)
intersect(p::Plane, l::Line) = intersect(l, p)


function getintersection{T}(l::Line{T}, s::Sphere{T})
	@assert isvalid(l)
	@assert isvalid(s)

	po = l.p[:, 1]
	vo = l.p[:, 2] - po
	co = po - s.c
	return quadroots(dot(vo, vo), 2dot(vo, co), dot(co, co)-s.r*s.r)
end

intersect{T}(l::Line{T}, s::Sphere{T}) = !isnan(getintersection(l, s)[1])
intersect(s::Sphere, l::Line) = intersect(l, s)


function getintersection{T}(l::Line{T}, ab::AABB{T})
	@assert isvalid(l)
	@assert isvalid(ab)

	tInt = (-inf(T), inf(T))
	for i = 1:3
		lo = l.p[i, 1]
		d = l.p[i, 2] - lo
		if iszero(d)
			# line is parallel to this box's dimension
			if ab.p[i, 1] <= lo <= ab.p[i, 2]
				continue
			else
				return nan(T), nan(T)
			end
		end
		dimInt = make_interval((ab.p[i, 1] - lo)/d, (ab.p[i, 2] - lo)/d)
		tInt = intersect_interval(tInt, dimInt)
		if empty_interval(tInt)
			return nan(T), nan(T)
		end
	end
	return tInt
end

intersect{T}(l::Line{T}, ab::AABB{T}) = !isnan(getintersection(l, ab)[1])
intersect(ab::AABB, l::Line) = intersect(l, ab)


# returns a line, a plane or nothing depending on the relative position of the planes
function getintersection{T}(p1::Plane{T}, p2::Plane{T})
	@assert isvalid(p1)
	@assert isvalid(p2)

	n1 = getnormal(p1)
	n2 = getnormal(p2)
	v = cross(n1, n2)
	anypt2 = getpoint(p2)
	if len2(v) < eps(T)
		#planes are parallel
		if iszero(getvalue(p1, anypt2))
			# a point on one plane lies on the other, so they are coincident
			return p1
		else
			# no intersection
			return nothing
		end
	end
	axis2 = cross(v, n2)
	lineOnP2 = Line(anypt2, anypt2+axis2)
	t = getintersection(lineOnP2, p1)
	@assert !isnan(t) && !isinf(t)
	ptIntersection = getpoint(lineOnP2, t)
	return Line(ptIntersection, ptIntersection + v)
end

intersect(p1::Plane, p2::Plane) = getintersection(p1, p2) != nothing


function outside{T}(c::Convex{T}, s::Sphere{T})
	@assert isvalid(c)
	@assert isvalid(s)
	for i = 1:size(c.planes, 2)
		if dot(c.planes[1:3, i], s.c) + c.planes[4, i] < -c.r
			return true
		end
	end
	return false
end

function outside{T}(c::Convex{T}, ab::AABB{T})
	@assert isvalid(c)
	@assert isvalid(ab)

	for i = 1:size(c.planes, 2)
		n = c.planes[1:3, i]
		# select the point on the box with the maximum value when substituted in the plane equation
		minPt = [n[j]<0 ? ab.p[j, 1] : ab.p[j, 2] for j=1:3]
		if dot(n, minPt) + c.planes[4, i] < 0
			return true
		end
	end
	return false
end

end
