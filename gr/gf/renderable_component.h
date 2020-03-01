#pragma once

#include "model.h"
#include "scene_object.h"

NAMESPACE_BEGIN(gf)

class RenderableComponent : public SceneObjectComponent {
	RTTR_ENABLE(SceneObjectComponent)
public:

public:
	std::shared_ptr<Model> _model;
	std::shared_ptr<Material> _material;
	
	std::shared_ptr<gr1::PipelineDrawCommand> _renderCmd;
};

NAMESPACE_END(gf)