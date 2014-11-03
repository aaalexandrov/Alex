function reld()
	GLFW.Terminate()
    reload("shapes.jl")
    reload("gr.jl")
    reload("simple.jl")
end

module Simple

import GLFW

import OGL
import GR
import Geom
import Math3D

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

global triangleMesh, simpleShader, gridTexture, triangleMaterial, triangleModel, diskMesh, diskModel

function initMeshes()
	triangleVert = [VertPosUV(-1, -1, 0, 0, 0), VertPosUV(1, -1, 0, 1, 0), VertPosUV(0, 1, 0, 0.5, 1)]
	triangleInd = Uint16[0, 1, 2]

	global triangleMesh = GR.Mesh()
	GR.init(triangleMesh, triangleVert, triangleInd, vert2pos)

	diskInd, diskPoints = Geom.regularpoly(3)
	diskVerts = Array(VertPosUV, size(diskPoints, 2))
	for i in 1:length(diskVerts)
		p = diskPoints[:, i]
		diskVerts[i] = VertPosUV(p[1], p[2], p[3], p[1], p[2])
	end
	global diskMesh = GR.Mesh()
	GR.init(diskMesh, diskVerts, diskInd, vert2pos)
end

function doneMeshes()
	GR.done(diskMesh)
	global diskMesh = nothing
	GR.done(triangleMesh)
	global triangleMesh = nothing
end

function initShaders()
	global simpleShader = GR.Shader()
	GR.init(simpleShader, "simple.vs", "simple.fs")
end

function doneShaders()
	GR.done(simpleShader)
	global simpleShader = nothing
end

function initTextures()
	global gridTexture = GR.Texture()
	GR.init(gridTexture, "data/grid2.png")
end

function doneTextures()
	GR.done(gridTexture)
	global gridTexture = nothing
end

function initModels()
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

clearColor = (0f0, 0f0, 1f0, 1f0)

function init()
	renderer = GR.Renderer()
	GR.init(renderer)
	GR.set_clear_color(renderer, clearColor)

	initShaders()
	initTextures()
	initMeshes()
	initModels()

	renderer
end

function done(renderer::GR.Renderer)
	doneModels()
	doneMeshes()
	doneTextures()
	doneShaders()

	GR.done(renderer)
end

function render(renderer::GR.Renderer)
	# GR.render(triangleMesh)
	# GR.render(triangleModel)
	# GR.render(diskModel)
	GR.add(renderer, diskModel)

	GR.render_frame(renderer)
end

const rotMatrix = Math3D.rotz(eye(Float32, 4), float32(pi / 18000))

function update()
	currentModel = GR.gettransform(diskModel)
	newModel = rotMatrix * currentModel
	GR.settransform(diskModel, newModel)
end

function setViewport(renderer::GR.Renderer, width::Integer, height::Integer)
	OGL.glViewport(0, 0, width, height)

	m = Math3D.ortho(2, 2height / width, -1f0, 1)
	# GR.setuniform(triangleMaterial, :projection, m)
	GR.setproj(GR.getcamera(renderer), m)
end

function openWindow()
	GLFW.Init()
	GLFW.WindowHint(GLFW.DEPTH_BITS, 24)
	GLFW.WindowHint(GLFW.STENCIL_BITS, 8)
	global window = GLFW.CreateWindow(640, 480, "Simple.jl")
	GLFW.MakeContextCurrent(window)
	GLFW.SwapInterval(0)

	# update opengl function pointers on windows
	OGL.updateGL()

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

		yield()
		frames += 1
	end

	info("Average FPS: $(frames / (time() - startTime))")

	done(renderer)

	GLFW.Terminate()
end

#@async openWindow()

end
