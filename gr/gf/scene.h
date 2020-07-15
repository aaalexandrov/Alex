#pragma once

#include "scene_object.h"
#include "gr1/utl/shader_param_data.h"

NAMESPACE_BEGIN(gf)

class Scene : public std::enable_shared_from_this<Scene> {
	RTTR_ENABLE()
public:

public:
	gr1::ShaderParamData _params;
};

NAMESPACE_END(gf)