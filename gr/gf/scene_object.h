#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"

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

public:
	std::vector<std::unique_ptr<SceneObjectComponent>> _components;
	glm::mat4 _transform;
	std::weak_ptr<Scene> _scene;
};

NAMESPACE_END(gf)