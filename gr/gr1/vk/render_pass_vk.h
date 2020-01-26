#pragma once

#include "../render_pass.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class RenderPassVk : public RenderPass {
	RTTR_ENABLE(RenderPass)
public:
	RenderPassVk(Device &device) : RenderPass(device) {}

		// TO REMOVE
	void Prepare() override { InitRenderPass(); }
	void Execute() override {}
protected:
	void InitRenderPass();

	static vk::AttachmentLoadOp GetLoadOpFromContent(ContentTreatment content);
	static vk::AttachmentStoreOp GetStoreOpFromContent(ContentTreatment content);

	vk::UniqueRenderPass _renderPass;
};

NAMESPACE_END(gr1)