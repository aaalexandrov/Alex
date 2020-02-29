#pragma once

#include "output_pass_vk.h"
#include "queue_vk.h"

NAMESPACE_BEGIN(gr1)

class PresentPassVk : public PresentPass, public PassVk {
	RTTR_ENABLE(PresentPass, PassVk)
public:
	PresentPassVk(Device &device);

	void Prepare() override;
	void Submit(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { return vk::PipelineStageFlagBits::eTransfer;	}
protected:
	vk::Result _presentResult;
	CmdBufferVk _cmdSignal;
	vk::UniqueSemaphore _beforePresent;
};

NAMESPACE_END(gr1)