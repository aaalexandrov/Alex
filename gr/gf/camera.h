#pragma once

#include "scene_object.h"

NAMESPACE_BEGIN(gf)

class Camera : public SceneObject {
	RTTR_ENABLE(SceneObject)
public:


public:
	glm::mat4 _projection;
};

NAMESPACE_END(gf)