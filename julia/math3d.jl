module Math3D

export normalize, normalize!, orthogonalize, orthogonalize!
export rotx, roty, rotz, rot, trans, perspective, ortho

function normalize(v::Vector) 
	l2 = sumabs2(v)
	l2 < eps() ? v : v / sqrt(l2)
end

function normalize!(v::Vector) 
	l2 = sumabs2(v)
	if l < eps()
		v
	else
		v /= sqrt(sumabs2(v))
	end
end	

function orthogonalize(v::Vector, n::Vector)
	nUnit = normalize(n)
	v - nUnit * dot(v, nUnit)
end

function orthogonalize!(v::Vector, n::Vector)
	nUnit = normalize(n)
	v -= nUnit * dot(v, nUnit)
end

function rotx(angle::Real, m::Matrix)
	s = sin(angle)
	c = cos(angle)
	m[1:3, 1] = [1, 0, 0]
	m[1:3, 2] = [0, c, -s]
	m[1:3, 3] = [0, s, c]
	return m
end

function roty(angle::Real, m::Matrix)
	s = sin(angle)
	c = cos(angle)
	m[1:3, 1] = [c, 0, -s]
	m[1:3, 2] = [0, 1, 0]
	m[1:3, 3] = [s, 0, c]
	return m
end

function rotz(angle::Real, m::Matrix)
	s = sin(angle)
	c = cos(angle)
	m[1:3, 1] = [c, s, 0]
	m[1:3, 2] = [-s, c, 0]
	m[1:3, 3] = [0, 0, 1]
	return m
end

rotx(angle::Real) = rotx(angle, Array(typeof(angle), 3, 3))
roty(angle::Real) = roty(angle, Array(typeof(angle), 3, 3))
rotz(angle::Real) = rotz(angle, Array(typeof(angle), 3, 3))

function rot(axis::Vector, angle::Real, m::Matrix)
	u = normalize(axis[1:3])
	s = sin(angle)
	c = cos(angle)
	m[1:3, 1] = [c+u[1]^2*(1-c),         u[2]*u[1]*(1-c)+u[3]*s, u[3]*u[1]*(1-c)-u[2]*s]
	m[1:3, 2] = [u[1]*u[2]*(1-c)-u[3]*s, c+u[2]^2*(1-c),         u[3]*u[2]*(1-c)+u[1]*s]
	m[1:3, 3] = [u[1]*u[3]*(1-c)+u[2]*s, u[2]*u[3]*(1-c)-u[1]*s, c+u[3]^2*(1-c)        ]
	return m
end

rot(axis::Vector, angle::Real) = rot(axis, angle, Array(typeof(angle), 3, 3))

function trans(t::Vector, m::Matrix)
	m[1:3, 4] = t[1:3]
	return m
end

trans(t::Vector) = trans(t, eye(eltype(t), 4))

function perspective(left::Real, right::Real, top::Real, bottom::Real, near::Real, far::Real, m::Matrix)
	m[1:4, 1] = [2near/(right-left),        0,                         0,                       0]
	m[1:4, 2] = [0,                         2near/(top-bottom),        0,                       0]
	m[1:4, 3] = [(right+left)/(right-left), (top+bottom)/(top-bottom), (-far-near)/(far-near), -1]
	m[1:4, 4] = [0,                         0,                         -2far*near/(far-near),   0]
	return m
end

perspective(left::Real, right::Real, top::Real, near::Real, far::Real) = perspective(left, right, top, bottom, near, far, Array(typeof(near), 4, 4))
perspective(width::Real, height::Real, near::Real, far::Real, m::Matrix) = perspective(-0.5width, 0.5width, -0.5height, 0.5height, near, far, m)
perspective(width::Real, height::Real, near::Real, far::Real) = perspective(width, height, near, far, Array(typeof(near), 4, 4))

function ortho(left::Real, right::Real, top::Real, bottom::Real, near::Real, far::Real, m::Matrix)
	m[1:4, 1] = [2/(right-left),             0,                          0,                      0]
	m[1:4, 2] = [0,                          2/(top-bottom),             0,                      0]
	m[1:4, 3] = [0,                          0,                          -2/(far-near),          0]
	m[1:4, 4] = [(-right-left)/(right-left), (-top-bottom)/(top-bottom), (-far-near)/(far-near), 1]
	return m
end

ortho(left::Real, right::Real, top::Real, bottom::Real, near::Real, far::Real) = ortho(left, right, top, bottom, near, far, Array(typeof(near), 4, 4))
ortho(width::Real, height::Real, near::Real, far::Real, m::Matrix) = ortho(-0.5width, 0.5width, -0.5height, 0.5height, near, far, m)
ortho(width::Real, height::Real, near::Real, far::Real) = ortho(width, height, near, far, Array(typeof(near), 4, 4))

end