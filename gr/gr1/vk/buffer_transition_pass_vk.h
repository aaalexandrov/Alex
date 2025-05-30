#pragma once

#include "output_pass_vk.h"
#include "queue_vk.h"

NAMESPACE_BEGIN(gr1)

class BufferTransitionPassVk : public ResourceStateTransitionPass, public PassVk {
	RTTR_ENABLE(ResourceStateTransitionPass, PassVk)
public:
	BufferTransitionPassVk(Device &device);

	void Prepare() override;
	void Submit(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override;

public:

	CmdBufferVk _srcCmds, _dstCmds;
	vk::UniqueSemaphore _queueTransitionSemaphore;
};


NAMESPACE_END(gr1)