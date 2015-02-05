module Simple

import GLFW

import ModernGL
import GR
import Geom
import Math3D
import SimpleCam

function rld()
	GLFW.Terminate()
	Base.reload("math3d.jl")
	Base.reload("geom.jl")
    Base.reload("shapes.jl")
    Base.reload("gr.jl")
    Base.reload("simplecam.jl")
    Base.reload("simple.jl")
end


import GR: Vec2, Vec3, Vec4, Matrix4

immutable VertPos
	position::Vec3
end

VertPos(x, y, z) = VertPos(Vec3(x, y, z))

immutable VertPosNorm
	position::Vec3
	normal::Vec3
end

VertPosNorm(x, y, z, nx, ny, nz) = VertPosNorm(Vec3(x, y, z), Vec3(nx, ny, nz))

immutable VertPosUV
	position::Vec3
	uv::Vec2
end

VertPosUV(x, y, z, u, v) = VertPosUV(Vec3(x, y, z), Vec2(u, v))

vert2pos(v) = [v.position.x, v.position.y, v.position.z]

global triangleMaterial, triangleModel, diskModel
global freeCam

function initMeshes(renderer::GR.Renderer)
	triangleVert = [VertPosUV(-1, -1, 0, 0, 0), VertPosUV(1, -1, 0, 1, 0), VertPosUV(0, 1, 0, 0.5, 1)]
	triangleInd = Uint16[0, 1, 2]

	GR.init(GR.Mesh(), renderer, triangleVert, triangleInd, vert2pos, id = :triangle)

	diskInd, diskPoints = Geom.regularpoly(5)
	diskVerts = Array(VertPosUV, size(diskPoints, 2))
	for i in 1:length(diskVerts)
		p = diskPoints[:, i]
		diskVerts[i] = VertPosUV(p[1], p[2], p[3], p[1], p[2])
	end

	GR.init(GR.Mesh(), renderer, diskVerts, diskInd, vert2pos, id = :disk)
end

function initShaders(renderer::GR.Renderer)
	GR.init(GR.Shader(), renderer, "data/simple")
end

global texName = "data/grid2.png"
#global texName = "fox.png"

function initTextures(renderer::GR.Renderer)
	GR.init(GR.Texture(), renderer, texName)
end

function initModels(renderer::GR.Renderer)
	simpleShader = GR.get_resource(renderer, symbol("data/simple"))
	gridTexture = GR.get_resource(renderer, symbol(texName))
	triangleMesh = GR.get_resource(renderer, :triangle)
	diskMesh = GR.get_resource(renderer, :disk)

	global triangleMaterial = GR.Material(simpleShader)

	ident = eye(Float32, 4) # identity matrix 4x4
	GR.setuniform(triangleMaterial, :projection, ident)
	GR.setuniform(triangleMaterial, :view, ident)
	GR.setuniform(triangleMaterial, :model, ident)

	GR.setuniform(triangleMaterial, :emissiveColor, (1f0, 1f0, 1f0, 1f0))
	GR.setuniform(triangleMaterial, :diffuseTexture, gridTexture)

	# GR.setstate(triangleMaterial, GR.AlphaBlendConstant((0.5f0, 0.5f0, 0.5f0, 0.5f0)))
	# GR.setstate(triangleMaterial, GR.AlphaBlendDisabled())

	global triangleModel = GR.Model(triangleMesh, triangleMaterial)
	global diskModel = GR.Model(diskMesh, triangleMaterial)
end

function doneModels()
	global triangleMaterial = nothing
	global triangleModel = nothing
	global diskModel = nothing
end

function initCamera(renderer::GR.Renderer)
	global window
	global freeCam = SimpleCam.FreeCamera(renderer, window, Float32[0.5, 0.5, 0.5], Float32[pi/8, pi/8, pi/8])
end

function doneCamera()
	global freeCam = nothing
end

clearColor = (0f0, 0f0, 1f0, 1f0)

function init()
	renderer = GR.Renderer()
	GR.init(renderer)
	GR.set_clear_color(renderer, clearColor)

	initShaders(renderer)
	initTextures(renderer)
	initMeshes(renderer)
	initModels(renderer)
	initCamera(renderer)

	renderer
end

function done(renderer::GR.Renderer)
	doneCamera()
	doneModels()

	GR.done(renderer)
end

function render(renderer::GR.Renderer)
	GR.add(renderer, diskModel)

	GR.render_frame(renderer)
end

global lastTime = time()

function update()
	global lastTime
	timeNow = time()
	deltaTime = timeNow - lastTime
	currentModel = GR.gettransform(diskModel)
	rotMatrix = Math3D.rotz(eye(Float32, 4), float32(deltaTime * pi / 2))
	newModel = rotMatrix * currentModel
	GR.settransform(diskModel, newModel)
	lastTime = timeNow
end

function process_input()
	global freeCam
	SimpleCam.process_input(freeCam)
end

function setViewport(renderer::GR.Renderer, width::Integer, height::Integer)
	ModernGL.glViewport(0, 0, width, height)

	# m = Math3D.ortho(2, 2height / width, -1f0, 1)
	m = Math3D.persp_horizontal_fov(pi/2, width / height, 0.1f0, 100, leftHanded = true)
	# m = Math3D.perspective(0.5, 0.5, 0.5f0, 10)
	# GR.setuniform(triangleMaterial, :projection, m)
	GR.setproj(renderer.camera, m)
	GR.settransform(renderer.camera, Math3D.trans(Float32[0, 0, -1]))
end

function openWindow()
	GLFW.Init()
	GLFW.WindowHint(GLFW.DEPTH_BITS, 24)
	GLFW.WindowHint(GLFW.STENCIL_BITS, 8)
	global window = GLFW.CreateWindow(640, 480, "Simple.jl")
	GLFW.MakeContextCurrent(window)
	GLFW.SwapInterval(0)

	renderer = init()

	setViewport(renderer, GLFW.GetFramebufferSize(window)...)
	GLFW.SetFramebufferSizeCallback(window, (win::GLFW.Window, width::Cint, height::Cint) -> setViewport(renderer, width, height))

	startTime = time()
	frames = 0

	while !GLFW.WindowShouldClose(window)
		render(renderer)
		update()

		GLFW.SwapBuffers(window)
		GLFW.PollEvents()

		process_input()

		yield()
		frames += 1
	end

	info("Average FPS: $(frames / (time() - startTime))")

	done(renderer)

	GLFW.Terminate()
end

#@async openWindow()

end
