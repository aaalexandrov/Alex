type Camera
	xform::Matrix{Float32}
	view::Matrix{Float32}
	proj::Matrix{Float32}
	frustum::Shapes.Convex{Float32}
	viewDirty::Bool
	frustumDirty::Bool

	Camera() = new(eye(Float32, 4), eye(Float32, 4), eye(Float32, 4), Convex(Float32, 6), false, true)
end

function setxform(cam::Camera, xform::Matrix{Float32})
	cam.xform[:,:] = xform
	cam.viewDirty = true
	cam.frustumDirty = true
end

function getview(cam::Camera)
	if cam.viewDirty
		cam.view[:,:] = inv(cam.xform)
		cam.viewDirty = false
	end
	cam.view
end

function setproj(cam::Camera, proj::Matrix{Float32})
	cam.proj[:,:] = proj
	cam.frustumDirty = true
end

function getfrustum(cam::Camera)
	if cam.frustimDirty
		calc_frustum(cam, cam.frustum.planes)
		cam.frustumDirty = false
	end
	cam.frustum
end

function normalized_plane(n, p)
	nn = n / len(n)
	[nn, -dot(p, nn)]
end

function calc_frustum(cam::Camera, dest::Matrix{Float32})
	ptMin = Float32[-1, -1, -1]
	ptMax = Float32[ 1,  1,  1]
	projSpaceFrust = hcat(normalized_plane(Float32[ 1,  0,  0], ptMin),
						  normalized_plane(Float32[ 0,  1,  0], ptMin),
						  normalized_plane(Float32[ 0,  0,  1], ptMin),
						  normalized_plane(Float32[-1,  0,  0], ptMax),
						  normalized_plane(Float32[ 0, -1,  0], ptMax),
						  normalized_plane(Float32[ 0,  0, -1], ptMax))

	dest[:,:] = At_mul_B(cam.proj * getview(cam), projSpaceFrust)
end
