module Sample

import GamEn
import GLFW
import GRU
import Math3D
import FTFont
import SimpleCam

function rld()
	GLFW.Terminate()
	reload("GRU.jl")
	reload("GamEn.jl")
	reload("SimpleCam")
	include("Sample.jl")
end

function onviewport(engine::GamEn.Engine, event::Symbol, width::Integer, height::Integer, font::GRU.Font)
	m = Math3D.persp_horizontal_fov(pi/2, width / height, 0.1f0, 100, leftHanded = true)
	GRU.setproj(engine.renderer.camera, m)
	GRU.settransform(engine.renderer.camera, Math3D.trans(Float32[0, 0, -1]))

	m = Math3D.ortho(0, width, 0, height, -1.0f0, 1.0f0)
	GRU.setuniform(font.model.material, :projection, m)
end

function onrender(engine::GamEn.Engine, event::Symbol, objects::Dict{Symbol, Any})
	for (k, v) in objects
		if isa(v, GRU.Renderable) && !isa(v, GRU.Font)
			GRU.add(engine.renderer, v)
		end
	end
	GRU.add(engine.renderer, objects[:font])
end

function onupdate(engine::GamEn.Engine, event::Symbol, objects::Dict{Symbol, Any})
	deltaTime = engine.timeNow - engine.timePrev
	sphere = objects[:sphere]
	currentModel = GRU.gettransform(sphere)
	rotMatrix = Math3D.rotz(eye(Float32, 4), Float32(deltaTime * pi / 2))
	newModel = rotMatrix * currentModel
	GRU.settransform(sphere, newModel)
	airplane = objects[:airplane]
	currentModel = GRU.gettransform(airplane)
	GRU.settransform(airplane, currentModel * rotMatrix)

	font = objects[:font]
	GRU.cleartext(font)
	cursor = FTFont.TextCursor(font.font.size.x, font.font.lineDistance)
	frameCount = objects[:frames]
	frameTimes = objects[:frameTimes]
	frames = length(frameTimes)
	curFrame = (frameCount-1)%frames+1
	nextFrame = frameCount%frames+1
	fps = frames / (frameTimes[curFrame] - frameTimes[nextFrame])
	frameTimes[nextFrame] = engine.timeNow
	objects[:frames] = frameCount + 1
	GRU.drawtext(font, cursor, "FPS: " * string(round(fps, 2)), (1f0, 1f0, 0f0, 1f0))
end

function oninput(engine::GamEn.Engine, event::Symbol, freeCam::SimpleCam.FreeCamera)
	SimpleCam.process_input(freeCam)
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

function load_defs(engine::GamEn.Engine)
	loaded = Dict{Symbol, Any}()
	loaded[:font] = GamEn.load_def(engine, "roboto")
	loaded[:triangle] = GamEn.load_def(engine, "model_triangle")
	loaded[:sphere] = GamEn.load_def(engine, "model_sphere")
	loaded[:airplane] = GamEn.load_def(engine, "model_cessna")
	loaded
end

function run()
	engine = GamEn.Engine("data")
	GamEn.init(engine)

	freeCam = SimpleCam.FreeCamera(engine.renderer, engine.window, Float32[2, 2, 2], Float32[pi/2, pi/2, pi/2])

	global loaded = load_defs(engine)
	loaded[:frameTimes] = fill(0.0, 100)
	loaded[:frames] = 1

	GamEn.add_event(engine, :viewport) do engine, event, width, height
		onviewport(engine, event, width, height, loaded[:font])
	end
	GamEn.add_event(engine, :render) do engine, event
		onrender(engine, event, loaded)
	end
	GamEn.add_event(engine, :update) do engine, event
		onupdate(engine, event, loaded)
	end
	GamEn.add_event(engine, :input) do engine, event
		oninput(engine, event, freeCam)
	end

	start = time()

	GamEn.run(engine)

	fps = loaded[:frames] / (time() - start)
	info("Total FPS: $fps")

	GamEn.done(engine)
end

end
