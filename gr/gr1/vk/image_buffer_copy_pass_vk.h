#pragma once

#include "output_pass_vk.h"
#include "queue_vk.h"

NAMESPACE_BEGIN(gr1)

class ImageBufferCopyPassVk : public ImageBufferCopyPass, public PassVk {
	RTTR_ENABLE(ImageBufferCopyPass, PassVk)
public:
	ImageBufferCopyPassVk(Device &device);

	void Prepare() override;
	void Submit(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { return vk::PipelineStageFlagBits::eTransfer; }

protected:
	CmdBufferVk _cmdCopy;
};

NAMESPACE_END(gr1)