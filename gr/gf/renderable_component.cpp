#include "renderable_component.h"

NAMESPACE_BEGIN(gf)

void RenderableComponent::SetModel(std::shared_ptr<Model> const &model)
{
	_model = model;
	_overrideMaterials.resize(_model->_materials.size());
	std::fill(_overrideMaterials.begin(), _overrideMaterials.end(), nullptr);
}

NAMESPACE_END(gf)