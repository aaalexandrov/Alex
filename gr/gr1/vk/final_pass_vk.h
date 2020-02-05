#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class FinalPassVk : public FinalPass, public PassVk {
	RTTR_ENABLE(FinalPass, PassVk)
public:
	FinalPassVk(Device &device);

	void Prepare() override;
	void Execute(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { ASSERT(0); return vk::PipelineStageFlagBits::eTopOfPipe; }

	bool IsFinished() override;
	void WaitToFinish() override;

protected:
	vk::UniqueFence _passesFinished;
	vk::UniqueCommandBuffer _cmdFinish;
};

NAMESPACE_END(gr1)