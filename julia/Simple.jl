module Simple

import GLFW

import ModernGL
import GR
import Geom
import Math3D
import SimpleCam
import OGLHelper
import FTFont

function rld()
	GLFW.Terminate()
	Base.reload("Math3D.jl")
	Base.reload("Geom.jl")
    Base.reload("Shapes.jl")
    Base.reload("GR.jl")
    Base.reload("SimpleCam.jl")
    Base.reload("Simple.jl")
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

immutable VertPosNormUV
	position::Vec3
	normal::Vec3
    uv::Vec2
end
VertPosNormUV(x, y, z, nx, ny, nz, u, v) = VertPosNormUV(Vec3(x, y, z), Vec3(nx, ny, nz), Vec2(u, v))

immutable VertPosColorUV
	position::Vec3
	color::Vec4
	uv::Vec2
end
VertPosColorUV(x, y, z, r, g, b, a, u, v) = VertPosColorUV(Vec3(x, y, z), Vec4(r, g, b, a), Vec2(u, v))

vert2pos(v) = [v.position.x, v.position.y, v.position.z]

global triangleMaterial, triangleModel, diskMaterial, diskModel
global freeCam
global font


function initMeshes(renderer::GR.Renderer)
	triangleVert = [VertPosUV(-1, -1, 0, 0, 0), VertPosUV(1, -1, 0, 1, 0), VertPosUV(0, 1, 0, 0.5, 1)]
	triangleInd = Uint16[0, 2, 1]
	GR.init(GR.Mesh(), renderer, triangleVert, triangleInd, vert2pos, id = :triangle)

	diskInd, diskPoints, diskNormals = Geom.sphere(10; smooth = false) # Geom.regularpoly(5)
    diskVerts = [VertPosNormUV(diskPoints[1, i], diskPoints[2, i], diskPoints[3, i],
                               diskNormals[1, i], diskNormals[2, i], diskNormals[3, i],
                               diskPoints[1, i], diskPoints[2, i])
                 for i in 1:size(diskPoints, 2)]
	GR.init(GR.Mesh(), renderer, diskVerts, diskInd, vert2pos, id = :disk)
end

function setup_matrices(model::GR.Model)
    GR.setuniform(model.material, :model, model.transform)
    camera = model.material.shader.renderer.camera
    GR.setuniform(model.material, :view, GR.getview(camera))
    GR.setuniform(model.material, :projection, GR.getproj(camera))
    if GR.hasuniform(model.material, :modelIT)
        GR.setuniform(model.material, :modelIT, transpose(inv(model.transform[1:3, 1:3])))
    end
end

function initShaders(renderer::GR.Renderer)
	GR.init(GR.Shader(), renderer, "data/simple", setup_matrices)
	GR.init(GR.Shader(), renderer, "data/font")
    GR.init(GR.Shader(), renderer, "data/diffuse", setup_matrices)
end

global texName = "data/Locator_Grid.png"

function initTextures(renderer::GR.Renderer)
	GR.init(GR.Texture(), renderer, texName)
end

function initFonts(renderer::GR.Renderer)
	fontShader = GR.get_resource(renderer, symbol("data/font"))

	FTFont.init()
	ftFont = FTFont.loadfont("data/roboto-regular.ttf"; sizeXY = (16, 16), chars = ['\u0000':'\u00ff', 'А':'Я', 'а':'я'])
	FTFont.done()

	ident = eye(Float32, 4)
    global font = GR.Font()
	GR.init(font, ftFont, fontShader, VertPosColorUV)
	GR.setuniform(font.model.material, :emissiveColor, Float32[1, 1, 1, 1])
	GR.setuniform(font.model.material, :view, ident)
    GR.setuniform(font.model.material, :model, ident)

#=	cursor = FTFont.TextCursor(ftFont.size.x, ftFont.lineDistance)
	GR.drawtext(font, cursor, "Try that for size", (1f0, 1f0, 0f0, 1f0))
	cursor = FTFont.TextCursor(ftFont.size.x, 2ftFont.lineDistance)
	GR.drawtext(font, cursor, "Kk фФ", (1f0, 0f0, 0f0, 1f0))
=#
end

function doneFonts()
	font = nothing
end

function initMaterials(renderer::GR.Renderer)
	simpleShader = GR.get_resource(renderer, symbol("data/simple"))
	gridTexture = GR.get_resource(renderer, symbol(texName))

	global triangleMaterial = GR.Material(simpleShader)
    GR.setstate(triangleMaterial, GR.CullStateCCW())
    GR.setstate(triangleMaterial, GR.DepthStateLess())

	ident = eye(Float32, 4) # identity matrix 4x4
	GR.setuniform(triangleMaterial, :projection, ident)
	GR.setuniform(triangleMaterial, :view, ident)
	GR.setuniform(triangleMaterial, :model, ident)

	GR.setuniform(triangleMaterial, :emissiveColor, [1f0, 1f0, 1f0, 1f0])
	GR.setuniform(triangleMaterial, :diffuseTexture, gridTexture)

	# GR.setstate(triangleMaterial, GR.AlphaBlendConstant((0.5f0, 0.5f0, 0.5f0, 0.5f0)))
	# GR.setstate(triangleMaterial, GR.AlphaBlendDisabled())

    diffuseShader = GR.get_resource(renderer, symbol("data/diffuse"))

    global diskMaterial = GR.Material(diffuseShader)
    GR.setstate(diskMaterial, GR.CullStateCCW())
    GR.setstate(diskMaterial, GR.DepthStateLess())

   	GR.setuniform(diskMaterial, :projection, ident)
	GR.setuniform(diskMaterial, :view, ident)
	GR.setuniform(diskMaterial, :model, ident)

    GR.setuniform(diskMaterial, :sunDirection, Math3D.normalize([1f0, 1f0, -1f0]))
    GR.setuniform(diskMaterial, :sunColor, [1f0, 1f0, 0.5f0] * 0.6f0)
    GR.setuniform(diskMaterial, :ambientColor, [0.4f0, 0.4f0, 0.4f0, 1f0])

    GR.setuniform(diskMaterial, :ambientMaterial, [1f0, 1f0, 1f0, 1f0])
    GR.setuniform(diskMaterial, :diffuseMaterial, [1f0, 1f0, 1f0])

	GR.setuniform(diskMaterial, :diffuseTexture, gridTexture)
end

function doneMaterials()
	global triangleMaterial = nothing
    global diskMaterial = nothing
end

function initModels(renderer::GR.Renderer)
    global triangleMaterial
	triangleMesh = GR.get_resource(renderer, :triangle)
	diskMesh = GR.get_resource(renderer, :disk)

	global triangleModel = GR.Model(triangleMesh, triangleMaterial)
    GR.settransform(triangleModel, Math3D.trans(Float32[0, 0, 1.5]))
	global diskModel = GR.Model(diskMesh, diskMaterial)
end

function doneModels()
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
	initFonts(renderer)
	initMeshes(renderer)
    initMaterials(renderer)
	initModels(renderer)
	initCamera(renderer)

	renderer
end

function done(renderer::GR.Renderer)
	doneCamera()
    doneModels()
	doneMaterials();

	GR.done(renderer)
end

function render(renderer::GR.Renderer)
	GR.add(renderer, diskModel)
    GR.add(renderer, triangleModel)
	GR.add(renderer, font)

	GR.render_frame(renderer)
end

global lastTime = time()
global frameCount = 1

function update()
	global lastTime
	timeNow = time()
	deltaTime = timeNow - lastTime
	currentModel = GR.gettransform(diskModel)
	rotMatrix = Math3D.rotz(eye(Float32, 4), float32(deltaTime * pi / 2))
	newModel = rotMatrix * currentModel
	GR.settransform(diskModel, newModel)

    if deltaTime > 0
        global font, frameCount
        GR.cleartext(font)
        cursor = FTFont.TextCursor(font.font.size.x, font.font.lineDistance)
        GR.drawtext(font, cursor, "FPS: " * string(round(frameCount / deltaTime, 2)), (1f0, 1f0, 0f0, 1f0))
        frameCount = 1
    else
        frameCount += 1
    end

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

	global font
	m = Math3D.ortho(0, width, 0, height, -1.0f0, 1.0f0)
	GR.setuniform(font.model.material, :projection, m)
end

function openWindow()
	GLFW.Init()
	GLFW.WindowHint(GLFW.DEPTH_BITS, 24)
	GLFW.WindowHint(GLFW.STENCIL_BITS, 8)
	GLFW.WindowHint(GLFW.CONTEXT_VERSION_MAJOR, 3)
	GLFW.WindowHint(GLFW.CONTEXT_VERSION_MINOR, 3)
	global window = GLFW.CreateWindow(640, 480, "Simple.jl")
	GLFW.MakeContextCurrent(window)
	GLFW.SwapInterval(0)

	renderer = init()

	OGLHelper.gl_info()

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

	info("Average FPS: $(round(frames / (time() - startTime), 2))")

	done(renderer)

	GLFW.Terminate()
end

#@async openWindow()

end