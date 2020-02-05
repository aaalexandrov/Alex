#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class ImageTransitionPassVk : public ResourceStateTransitionPass, public PassVk {
	RTTR_ENABLE(ResourceStateTransitionPass, PassVk)
public:
	ImageTransitionPassVk(Device &device);

	void Prepare() override;
	void Execute(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override;
public:

	vk::UniqueCommandBuffer _srcCmds, _dstCmds;
	vk::UniqueSemaphore _queueTransitionSemaphore;
};

NAMESPACE_END(gr1)