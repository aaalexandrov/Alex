#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"
#include "util/geom.h"

NAMESPACE_BEGIN(gf)

class SceneObject;

class SceneObjectComponent {
	RTTR_ENABLE()
public:

public:
	SceneObject *_owner;
};

class Scene;
class SceneObject : public std::enable_shared_from_this<SceneObject> {
	RTTR_ENABLE()
public:

	void AddComponent(std::unique_ptr<SceneObjectComponent> &&component);

public:
	std::vector<std::unique_ptr<SceneObjectComponent>> _components;
	std::weak_ptr<Scene> _scene;
	util::Transform3F _transform;
};

NAMESPACE_END(gf)