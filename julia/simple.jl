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

global triangleMesh, simpleShader, gridTexture, triangleMaterial, triangleModel, diskMesh, diskModel

function initMeshes()
	triangleVert = [VertPosUV(-1, -1, 0, 0, 0), VertPosUV(1, -1, 0, 1, 0), VertPosUV(0, 1, 0, 0.5, 1)]
	triangleInd = Uint16[0, 1, 2]

	global triangleMesh = GR.Mesh()
	GR.init(triangleMesh, triangleVert, triangleInd)

	diskInd, diskPoints = Geom.regularpoly(3)
	diskVerts = Array(VertPosUV, size(diskPoints, 2))
	for i in 1:length(diskVerts)
		p = diskPoints[:, i]
		diskVerts[i] = VertPosUV(p[1], p[2], p[3], p[1], p[2])
	end
	global diskMesh = GR.Mesh()
	GR.init(diskMesh, diskVerts, diskInd)
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

function init()
	initShaders()
	initTextures()
	initMeshes()
	initModels()
end

function done()
	doneModels()
	doneMeshes()
	doneTextures()
	doneShaders()
end

function render()
	# GR.render(triangleMesh)
	# GR.render(triangleModel)
	GR.render(diskModel)
end

const rotMatrix = Math3D.rotz(float32(pi / 18000), eye(Float32, 4))

function update()
	currentModel = GR.gettransform(diskModel)
	newModel = rotMatrix * currentModel
	GR.settransform(diskModel, newModel)
end

clearColor = (0f0, 0f0, 1f0, 1f0)

function setClearColor(r, g, b, a = 1f0)
	global clearColor = (convert(Float32, r), convert(Float32, g), convert(Float32, b), convert(Float32, a))
end

function setViewport(width::Integer, height::Integer)
	OGL.glViewport(0, 0, width, height)

	m = Math3D.ortho(2, 2height / width, -1f0, 1)
	GR.setuniform(triangleMaterial, :projection, m)
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

	init()

	setViewport(GLFW.GetFramebufferSize(window)...)
	GLFW.SetFramebufferSizeCallback(window, (win::GLFW.Window, width::Cint, height::Cint) -> setViewport(width, height))

	startTime = time()
	frames = 0

	while !GLFW.WindowShouldClose(window)
		OGL.gl_clear_buffers(clearColor, 0, 0)

		render()
		update()

		GLFW.SwapBuffers(window)
		GLFW.PollEvents()

		yield()
		frames += 1
	end

	info("Average FPS: $(frames / (time() - startTime))")

	done()

	GLFW.Terminate()
end

#@async openWindow()

end
