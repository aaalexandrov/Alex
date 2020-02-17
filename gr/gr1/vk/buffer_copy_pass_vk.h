#pragma once

#include "output_pass_vk.h"
#include "buffer_vk.h"

NAMESPACE_BEGIN(gr1)

class BufferCopyPassVk : public BufferCopyPass, public PassVk {
	RTTR_ENABLE(BufferCopyPass, PassVk)
public:
	BufferCopyPassVk(Device &device);

	void Prepare() override;
	void Execute(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { return vk::PipelineStageFlagBits::eTransfer; }

protected:
	CmdBufferVk _cmdCopy;
};

NAMESPACE_END(gr1)