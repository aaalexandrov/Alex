#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class PresentPassVk : public PresentPass, public PassVk {
	RTTR_ENABLE(PresentPass, PassVk)
public:
	PresentPassVk(Device &device);

	void Prepare(PassData *passData) override;
	void Execute(PassData *passData) override;

protected:
	vk::Result _presentResult;
	vk::UniqueCommandBuffer _cmdSignal;
	vk::UniqueSemaphore _beforePresent;
};

NAMESPACE_END(gr1)