module Simple

import GLFW
import ModernGL

import GRU
import GLHelper
import Geom
import Math3D
import FTFont
import SimpleCam

function rld()
    GLFW.Terminate()
    reload("GRU.jl")
    include("SimpleCam.jl")
    include("Simple.jl")
end


global triangleMaterial, triangleModel, diskMaterial, diskModel
global freeCam
global font


function initMeshes(renderer::GRU.Renderer)
    simpleShader = GRU.get_resource(renderer, Symbol("data/simple"))
    trianglePos = Float32[-1  1  0;
                           -1 -1  1;
                           0  0  0]
    triangleUV = Float32[0 1 0.5;
                         0 0   1]
    triangleInd = UInt16[0, 2, 1]
    GRU.init(GRU.Mesh(), simpleShader, Dict{Symbol, Array}([(:position, trianglePos), (:texCoord, triangleUV)]), triangleInd; positionFunc = GRU.position_func(:position), id = :triangle)

    diskInd, diskPoints, diskNormals = Geom.sphere(10; smooth = false) # Geom.regularpoly(5)
    diffuseShader = GRU.get_resource(renderer, Symbol("data/diffuse"))
    GRU.init(GRU.Mesh(), diffuseShader, Dict{Symbol, Array}([(:position, diskPoints), (:norm, diskNormals), (:texCoord, diskPoints[1:2, :])]), diskInd; positionFunc = GRU.position_func(:position), id = :disk)
end

function setup_matrices(model::GRU.Model)
    GRU.setuniform(model.material, :model, model.transform)
    camera = model.material.shader.renderer.camera
    GRU.setuniform(model.material, :view, GRU.getview(camera))
    GRU.setuniform(model.material, :projection, GRU.getproj(camera))
    if GRU.hasuniform(model.material, :modelIT)
        GRU.setuniform(model.material, :modelIT, transpose(inv(model.transform[1:3, 1:3])))
    end
end

function initShaders(renderer::GRU.Renderer)
    GRU.init(GRU.Shader(), renderer, "data/simple", setup_matrices)
    GRU.init(GRU.Shader(), renderer, "data/font")
    GRU.init(GRU.Shader(), renderer, "data/diffuse", setup_matrices)
end

#global texName = "data/Locator_Grid.png"
global texName = "data/grid2.png"

function initTextures(renderer::GRU.Renderer)
    GRU.init(GRU.Texture(), renderer, texName)
end

function initFonts(renderer::GRU.Renderer)
    fontShader = GRU.get_resource(renderer, Symbol("data/font"))

    FTFont.init()
    ftFont = FTFont.loadfont("data/Roboto-Regular.ttf"; sizeXY = (16, 16), chars = ['\u0000':'\u00ff'; 'А':'Я'; 'а':'я'])
    FTFont.done()

    ident = eye(Float32, 4)
    global font = GRU.Font()
    GRU.init(font, ftFont, fontShader)
    GRU.setuniform(font.model.material, :emissiveColor, Float32[1, 1, 1, 1])
    GRU.setuniform(font.model.material, :view, ident)
    GRU.setuniform(font.model.material, :model, ident)
end

function doneFonts()
    font = nothing
end

function initMaterials(renderer::GRU.Renderer)
    simpleShader = GRU.get_resource(renderer, Symbol("data/simple"))
    gridTexture = GRU.get_resource(renderer, Symbol(texName))

    global triangleMaterial = GRU.Material(simpleShader)
    GRU.setstate(triangleMaterial, GRU.CullStateCCW())
    GRU.setstate(triangleMaterial, GRU.DepthStateLess())
    GRU.setstate(triangleMaterial, GRU.AlphaBlendDisabled())

    ident = eye(Float32, 4) # identity matrix 4x4
    GRU.setuniform(triangleMaterial, :projection, ident)
    GRU.setuniform(triangleMaterial, :view, ident)
    GRU.setuniform(triangleMaterial, :model, ident)

    GRU.setuniform(triangleMaterial, :emissiveColor, [1f0, 1f0, 1f0, 1f0])
    GRU.setuniform(triangleMaterial, :diffuseTexture, gridTexture)

    # GRU.setstate(triangleMaterial, GRU.AlphaBlendConstant((0.5f0, 0.5f0, 0.5f0, 0.5f0)))
    # GRU.setstate(triangleMaterial, GRU.AlphaBlendDisabled())

    diffuseShader = GRU.get_resource(renderer, Symbol("data/diffuse"))

    global diskMaterial = GRU.Material(diffuseShader)
    GRU.setstate(diskMaterial, GRU.CullStateCCW())
    GRU.setstate(diskMaterial, GRU.DepthStateLess())
    GRU.setstate(diskMaterial, GRU.AlphaBlendDisabled())

    GRU.setuniform(diskMaterial, :projection, ident)
    GRU.setuniform(diskMaterial, :view, ident)
    GRU.setuniform(diskMaterial, :model, ident)

    GRU.setuniform(diskMaterial, :sunDirection, Math3D.normalize([1f0, 1f0, -1f0]))
    GRU.setuniform(diskMaterial, :sunColor, [1f0, 1f0, 0.5f0] * 0.6f0)
    GRU.setuniform(diskMaterial, :ambientColor, [0.4f0, 0.4f0, 0.4f0, 1f0])

    GRU.setuniform(diskMaterial, :ambientMaterial, [1f0, 1f0, 1f0, 1f0])
    GRU.setuniform(diskMaterial, :diffuseMaterial, [1f0, 1f0, 1f0])

    GRU.setuniform(diskMaterial, :diffuseTexture, gridTexture)
end

function doneMaterials()
    global triangleMaterial = nothing
    global diskMaterial = nothing
end

function initModels(renderer::GRU.Renderer)
    global triangleMaterial
    triangleMesh = GRU.get_resource(renderer, :triangle)
    diskMesh = GRU.get_resource(renderer, :disk)

    global triangleModel = GRU.Model(triangleMesh, triangleMaterial)
    GRU.settransform(triangleModel, Math3D.trans(Float32[0, 0, 1.5]))
    global diskModel = GRU.Model(diskMesh, diskMaterial)
end

function doneModels()
    global triangleModel = nothing
    global diskModel = nothing
end

function initCamera(renderer::GRU.Renderer)
    global window
    global freeCam = SimpleCam.FreeCamera(renderer, window, Float32[2, 2, 2], Float32[pi/2, pi/2, pi/2])
end

function doneCamera()
    global freeCam = nothing
end

clearColor = (0f0, 0f0, 1f0, 1f0)

function init()
    renderer = GRU.Renderer()
    GRU.init(renderer)
    GRU.set_clear_color(renderer, clearColor)

    initShaders(renderer)
    initTextures(renderer)
    initFonts(renderer)
    initMeshes(renderer)
    initMaterials(renderer)
    initModels(renderer)
    initCamera(renderer)

    renderer
end

function done(renderer::GRU.Renderer)
    doneCamera()
    doneModels()
    doneMaterials();

    GRU.done(renderer)
end

function render(renderer::GRU.Renderer)
    GRU.add(renderer, diskModel)
    GRU.add(renderer, triangleModel)
    GRU.add(renderer, font)

    GRU.render_frame(renderer)
end

global lastTime = time()
global frameCount = 1

function update()
    global lastTime
    timeNow = time()
    deltaTime = timeNow - lastTime
    currentModel = GRU.gettransform(diskModel)
    rotMatrix = Math3D.rotz(eye(Float32, 4), Float32(deltaTime * pi / 2))
    newModel = rotMatrix * currentModel
    GRU.settransform(diskModel, newModel)

    if deltaTime > 0
        global font, frameCount
        GRU.cleartext(font)
        cursor = FTFont.TextCursor(font.font.size.x, font.font.lineDistance)
        GRU.drawtext(font, cursor, "FPS: " * string(round(frameCount / deltaTime, 2)), (1f0, 1f0, 0f0, 1f0))
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

function setViewport(renderer::GRU.Renderer, width::Integer, height::Integer)
    ModernGL.glViewport(0, 0, width, height)

    # m = Math3D.ortho(2, 2height / width, -1f0, 1)
    m = Math3D.persp_horizontal_fov(pi/2, width / height, 0.1f0, 100, leftHanded = true)
    # m = Math3D.perspective(0.5, 0.5, 0.5f0, 10)
    # GRU.setuniform(triangleMaterial, :projection, m)
    GRU.setproj(renderer.camera, m)
    GRU.settransform(renderer.camera, Math3D.trans(Float32[0, 0, -1]))

    global font
    m = Math3D.ortho(0, width, 0, height, -1.0f0, 1.0f0)
    GRU.setuniform(font.model.material, :projection, m)
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

    GLHelper.gl_info()

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
