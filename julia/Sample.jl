module Sample

import GameEn

function onviewport(engine::GamEn.Engine, event::Symbol, width::Integer, height::Integer)
	m = Math3D.persp_horizontal_fov(pi/2, width / height, 0.1f0, 100, leftHanded = true)
	GRU.setproj(engine.renderer.camera, m)
	GRU.settransform(engine.renderer.camera, Math3D.trans(Float32[0, 0, -1]))

	global font
	m = Math3D.ortho(0, width, 0, height, -1.0f0, 1.0f0)
	GRU.setuniform(font.model.material, :projection, m)
end

function onupdate(engine::GamEn.Engine, event::Symbol)
end

function oninput(engine::GamEn.Engine, event::Symbol)
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
	GamEn.load_def(engine, "")

end

function run()
	engine = GamEn.Engine("data")
	GamEn.init(engine)

	GamEn.add_event(engine, :viewport, onviewport)
	GamEn.add_event(engine, :update, onupdate)
	GamEn.add_event(engine, :input, oninput)

	load_defs(engine)

	GamEn.run(engine)

	GamEn.done(engine)
end

end
