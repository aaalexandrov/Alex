#pragma once

#include "resource.h"

NAMESPACE_BEGIN(gr1)

class RenderState : public Resource {
	RTTR_ENABLE(Resource)
public:
	RenderState(Device &device) : Resource(device) {}
};

NAMESPACE_END(gr1)

