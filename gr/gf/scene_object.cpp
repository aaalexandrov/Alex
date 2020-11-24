#include "scene_object.h"
#include "scene.h"

NAMESPACE_BEGIN(gf)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<SceneObjectComponent>("SceneObjectComponent")
		.constructor<SceneObject *>()(policy::ctor::as_raw_ptr);

	registration::class_<SceneObject>("SceneObject")
		.constructor<Scene *>()(policy::ctor::as_raw_ptr);
}


SceneObjectComponent::SceneObjectComponent(SceneObject *owner)
	: _owner(owner)
{
}

SceneObject::SceneObject(Scene *scene)
	: _scene(std::weak_ptr<Scene>(scene ? scene->shared_from_this() : nullptr))
{
}

SceneObjectComponent *SceneObject::AddComponent(rttr::type componentType)
{
	rttr::variant inst = componentType.create({ this });
	SceneObjectComponent *component = inst.get_value<SceneObjectComponent *>();
	ASSERT(component);
	_components.push_back(std::move(std::unique_ptr<SceneObjectComponent>(component)));
	return component;
}

void SceneObject::RemoveComponent(SceneObjectComponent *component)
{
	int32_t ind = GetComponentIndex(component);
	if (ind >= 0)
		_components.erase(_components.begin() + ind);
}

SceneObjectComponent *SceneObject::GetComponent(rttr::type componentType, SceneObjectComponent *prevComponent) const
{
	for (int32_t i = GetComponentIndex(prevComponent) + 1; i < _components.size(); ++i) {
		if (_components[i]->get_type().is_derived_from(componentType))
			return _components[i].get();
	}
	return nullptr;
}

int32_t SceneObject::GetComponentIndex(SceneObjectComponent *component) const
{
	auto const &found = std::find_if(_components.begin(), _components.end(), [component](auto &cmp) { return component == cmp.get(); });
	return found == _components.end() ? -1 : static_cast<int32_t>(std::distance(_components.begin(), found));
}


NAMESPACE_END(gf)

