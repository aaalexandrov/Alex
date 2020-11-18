#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"
#include "util/geom.h"

NAMESPACE_BEGIN(gf)

class SceneObject;

class SceneObjectComponent {
	RTTR_ENABLE()
public:

	SceneObjectComponent(SceneObject *owner);

public:
	SceneObject *_owner;
};

class Scene;
class SceneObject : public std::enable_shared_from_this<SceneObject> {
	RTTR_ENABLE()
public:

	SceneObject(Scene *scene);

	SceneObjectComponent *AddComponent(rttr::type componentType);
	template <typename CmpType>
	CmpType *AddComponent() { return static_cast<CmpType*>(AddComponent(rttr::type::get<CmpType>())); }
	void RemoveComponent(SceneObjectComponent *component);

	int32_t GetNumComponents() const { return static_cast<int32_t>(_components.size()); }
	SceneObjectComponent *GetComponent(int32_t cmpInd) const { return _components[cmpInd].get(); }

	SceneObjectComponent *GetComponent(rttr::type componentType, SceneObjectComponent *prevComponent = nullptr) const;
	template <typename CmpType>
	CmpType *GetComponent(CmpType *prevComponent = nullptr) { return static_cast<CmpType*>(GetComponent(rttr::type::get<CmpType>(), prevComponent)); }

	int32_t GetComponentIndex(SceneObjectComponent *component) const;
public:
	std::vector<std::unique_ptr<SceneObjectComponent>> _components;
	std::weak_ptr<Scene> _scene;
	util::Transform3F _transform;
};

NAMESPACE_END(gf)