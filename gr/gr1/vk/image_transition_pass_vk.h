#pragma once

#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class ImageTransitionPassVk : public ResourceStateTransitionPass, public PassVk {
	RTTR_ENABLE(ResourceStateTransitionPass, PassVk)
public:
	ImageTransitionPassVk(Device &device);

	void Prepare(PassData *passData) override;
	void Execute(PassData *passData) override;

public:
	vk::UniqueCommandBuffer _cmdTransition;
};

NAMESPACE_END(gr1)