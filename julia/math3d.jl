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

function rotx(m::Matrix, angle::Real)
	s = sin(angle)
	c = cos(angle)
	m[1,1] = 1
	m[2,1] = m[3,1] = m[1,2] = m[1,3] = 0
	m[2,2] = m[3,3] = c
	m[3,2] = -s
	m[2,3] = s
	return m
end

function roty(m::Matrix, angle::Real)
	s = sin(angle)
	c = cos(angle)
	m[1,1] = m[3,3] = c
	m[2,1] = m[1,2] = m[3,2] = m[2,3] = 0
	m[2,2] = 1
	m[3,1] = -s
	m[1,3] = s
	return m
end

function rotz(m::Matrix, angle::Real)
	s = sin(angle)
	c = cos(angle)
	m[1,1] = m[2,2] = c
	m[2,1] = s
	m[1,2] = -s
	m[3,1] = m[3,2] = m[1,3] = m[2,3] = 0
	m[3,3] = 1
	return m
end

# rotation with angles around each of the axes
function rotxyz(m::Matrix, xAngle::Real, yAngle::Real, zAngle::Real)
	# Tait-Bryan angles transform in the order X1Y2Z3
	sx = sin(xAngle)
	cx = cos(xAngle)
	sy = sin(yAngle)
	cy = cos(yAngle)
	sz = sin(zAngle)
	cz = cos(zAngle)

	m[1,1] = cy*cz
	m[2,1] = cx*sz+cz*sx*sy
	m[3,1] = sx*sz-cx*cz*sy

	m[1,2] = -cy*sz
	m[2,2] = cx*cz-sx*sy*sz
	m[3,2] = cz*sx+cx*sy*sz

	m[1,3] = sy
	m[2,3] = -cy*sx
	m[3,3] = cx*cy

	return m
end

rotx(angle::Real) = rotx(Array(typeof(angle), 3, 3), angle)
roty(angle::Real) = roty(Array(typeof(angle), 3, 3), angle)
rotz(angle::Real) = rotz(Array(typeof(angle), 3, 3), angle)
rotxyz(xAngle::Real, yAngle::Real, zAngle::Real) = rotxyz(Array(typeof(xAngle), 3, 3), xAngle, yAngle, zAngle)

function rot(m::Matrix, axis::Vector, angle::Real)
	u = normalize(axis[1:3])
	ux = u[1]
	uy = u[2]
	uz = u[3]
	s = sin(angle)
	c = cos(angle)

	m[1,1] = c+ux*ux*(1-c)
	m[2,1] = uy*ux*(1-c)+uz*s
	m[3,1] = uz*ux*(1-c)-uy*s

	m[1,2] = ux*uy*(1-c)-uz*s
	m[2,2] = c+uy*uy*(1-c) 
	m[3,2] = uz*uy*(1-c)+ux*s

	m[1,3] = ux*uz*(1-c)+uy*s
	m[2,3] = uy*uz*(1-c)-ux*s
	m[3,3] = c+uz*uz*(1-c)

	return m
end

rot(axis::Vector, angle::Real) = rot(Array(typeof(angle), 3, 3), axis, angle)

function trans(m::Matrix, t::Vector)
	m[1,4] = t[1]
	m[2,4] = t[2]
	m[3,4] = t[3]
	return m
end


trans(t::Vector) = trans(eye(eltype(t), 4), t)

function perspective(m::Matrix, left::Real, right::Real, top::Real, bottom::Real, near::Real, far::Real; leftHanded = false)
	zsign = leftHanded? -1 : 1
	m[1:4, 1:4] = [ 2near/(right-left)                   0  (right+left)/(right-left)*zsign                      0 ;
					                 0  2near/(top-bottom)  (top+bottom)/(top-bottom)*zsign                      0 ;
					                 0                   0     (-far-near)/(far-near)*zsign  -2far*near/(far-near) ;
					                 0                   0                         -1*zsign                      0 ]
	return m
end

perspective(left::Real, right::Real, top::Real, near::Real, far::Real; leftHanded = false) = perspective(Array(typeof(near), 4, 4), left, right, top, bottom, near, far, leftHanded = leftHanded)
perspective(m::Matrix, width::Real, height::Real, near::Real, far::Real; leftHanded = false) = perspective(m, -0.5width, 0.5width, -0.5height, 0.5height, near, far, leftHanded = leftHanded)
perspective(width::Real, height::Real, near::Real, far::Real; leftHanded = false) = perspective(Array(typeof(near), 4, 4), width, height, near, far, leftHanded = leftHanded)
function persp_horizontal_fov(m::Matrix, hfov::Real, w_h_ratio::Real, near::Real, far::Real; leftHanded = false)
	width = 2near*tan(hfov/2)
	perspective(m, width, width / w_h_ratio, near, far, leftHanded = leftHanded)
end
persp_horizontal_fov(hfov::Real, w_h_ratio::Real, near::Real, far::Real; leftHanded = false) = persp_horizontal_fov(Array(typeof(near), 4, 4), hfov, w_h_ratio, near, far, leftHanded = leftHanded)
function persp_vertical_fov(m::Matrix, vfov::Real, w_h_ratio::Real, near::Real, far::Real; leftHanded = false)
	height = 2near*tan(vfov/2)
	perspective(m, w_h_ratio * height, height, near, far, leftHanded = leftHanded)
end
persp_vertical_fov(vfov::Real, w_h_ratio::Real, near::Real, far::Real; leftHanded = false) = persp_vertical_fov(Array(typeof(near), 4, 4), vfov, w_h_ratio, near, far, leftHanded = leftHanded)

function ortho(m::Matrix, left::Real, right::Real, top::Real, bottom::Real, near::Real, far::Real; leftHanded = false)
	zsign = leftHanded? -1 : 1
	m[1:4, 1:4] = [ 2/(right-left)               0                   0 (-right-left)/(right-left) ;
					             0  2/(top-bottom)                   0 (-top-bottom)/(top-bottom) ;
					             0               0 -2/(far-near)*zsign     (-far-near)/(far-near) ;
					             0               0                   0                          1 ]
	return m
end

ortho(left::Real, right::Real, top::Real, bottom::Real, near::Real, far::Real; leftHanded = false) = ortho(Array(typeof(near), 4, 4), left, right, top, bottom, near, far, leftHanded = leftHanded)
ortho(m::Matrix, width::Real, height::Real, near::Real, far::Real; leftHanded = false) = ortho(m, -0.5width, 0.5width, -0.5height, 0.5height, near, far, leftHanded = leftHanded)
ortho(width::Real, height::Real, near::Real, far::Real; leftHanded = false) = ortho(Array(typeof(near), 4, 4), width, height, near, far, leftHanded = leftHanded)

end
