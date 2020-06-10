#pragma once

#include "scene_object.h"
#include "gr1/buffer.h"

NAMESPACE_BEGIN(gf)

class Scene : public std::enable_shared_from_this<Scene> {
public:

public:
	std::shared_ptr<gr1::Buffer> _perSceneUniforms;
};

NAMESPACE_END(gf)