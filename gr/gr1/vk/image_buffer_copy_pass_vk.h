#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class ImageBufferCopyPassVk : public ImageBufferCopyPass, public PassVk {
	RTTR_ENABLE(ImageBufferCopyPass, PassVk)
public:
	ImageBufferCopyPassVk(Device &device);

	void Prepare() override;
	void Execute(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { return vk::PipelineStageFlagBits::eTransfer; }

protected:
	vk::UniqueCommandBuffer _cmdCopy;
};

NAMESPACE_END(gr1)