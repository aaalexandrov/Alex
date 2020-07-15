#pragma once

#include "buffer_content.h"
#include "../execution_queue.h"
#include "../render_commands.h"

NAMESPACE_BEGIN(gr1)

class ShaderParamData {
	RTTR_ENABLE()
public:

public:
	BufferContent _uniforms;
	std::unordered_map<util::StrId, SamplerData> _textures;
};

NAMESPACE_END(gr1)