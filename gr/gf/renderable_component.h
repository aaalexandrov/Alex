#pragma once

#include "model.h"
#include "scene_object.h"

NAMESPACE_BEGIN(gf)

class RenderableComponent : public SceneObjectComponent {
	RTTR_ENABLE(SceneObjectComponent)
public:

	void SetModel(std::shared_ptr<Model> const &model);

public:
	std::shared_ptr<Model> _model;
	std::vector<std::shared_ptr<Material>> _overrideMaterials;
};

NAMESPACE_END(gf)