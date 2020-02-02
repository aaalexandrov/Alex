#pragma once

#include "../render_pass.h"
#include "output_pass_vk.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class RenderPassVk : public RenderPass, public PassVk {
	RTTR_ENABLE(RenderPass, PassVk)
public:
	RenderPassVk(Device &device);

	void ClearAttachments() override;
	int AddAttachment(ContentTreatment inputContent, ContentTreatment outputContent, glm::vec4 clearValue = glm::vec4()) override;
	void SetAttachmentImage(int attachmentIndex, std::shared_ptr<Image> const &img) override;

	void Prepare(PassData *passData) override;
	void Execute(PassData *passData) override;
protected:
	void InitRenderPass();
	void InitFramebuffer();

	void RecordPassCommands();
	void RecordRenderCommands();

	static vk::AttachmentLoadOp GetLoadOpFromContent(ContentTreatment content);
	static vk::AttachmentStoreOp GetStoreOpFromContent(ContentTreatment content);

	vk::UniqueRenderPass _renderPass;
	vk::UniqueFramebuffer _framebuffer;

	vk::UniqueCommandBuffer _passCmds;
};

NAMESPACE_END(gr1)