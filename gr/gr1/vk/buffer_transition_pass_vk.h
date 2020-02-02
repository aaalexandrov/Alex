#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class BufferTransitionPassVk : public ResourceStateTransitionPass, public PassVk {
	RTTR_ENABLE(ResourceStateTransitionPass, PassVk)
public:
	BufferTransitionPassVk(Device &device);

	void Prepare(PassData *passData) override;
	void Execute(PassData *passData) override;

public:

	vk::UniqueCommandBuffer _srcCmds, _dstCmds;
	vk::UniqueSemaphore _queueTransitionSemaphore;
};


NAMESPACE_END(gr1)