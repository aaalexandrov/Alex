#pragma once

#include "output_pass_vk.h"
#include "buffer_vk.h"

NAMESPACE_BEGIN(gr1)

class BufferCopyPassVk : public BufferCopyPass, public PassVk {
	RTTR_ENABLE(BufferCopyPass, PassVk)
public:
};

NAMESPACE_END(gr1)