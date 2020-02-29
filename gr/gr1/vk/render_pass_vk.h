#pragma once

#include "../render_pass.h"
#include "output_pass_vk.h"
#include "pipeline_store.h"
#include "queue_vk.h"

NAMESPACE_BEGIN(gr1)

class CommandPrepareInfoVk : public CommandPrepareInfo {
	RTTR_ENABLE()
public:
	CommandPrepareInfoVk(RenderPassVk *renderPassVk);

	vk::CommandBufferInheritanceInfo _cmdInheritInfo;
	vk::Viewport _viewport;
};

class CommandRecordInfoVk : public CommandRecordInfo {
	RTTR_ENABLE(CommandRecordInfo)
public:
	std::vector<vk::CommandBuffer> *_secondaryCmds;
};

class RenderPassVk : public RenderPass, public PassVk {
	RTTR_ENABLE(RenderPass, PassVk)
public:
	RenderPassVk(Device &device);

	void ClearAttachments() override;
	int AddAttachment(ContentTreatment inputContent, ContentTreatment outputContent, glm::vec4 clearValue = glm::vec4()) override;
	void SetAttachmentImage(int attachmentIndex, std::shared_ptr<Image> const &img) override;

	void Prepare() override;
	void Submit(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { return vk::PipelineStageFlagBits::eColorAttachmentOutput; }

	vk::RenderPass GetVkRenderPass() { return *_renderPass; }
	uint32_t GetSubpass() { return 0; }
protected:
	void InitRenderPass();
	void InitFramebuffer();

	void RecordPassCommands();

	void PrepareToRecordRenderCommands();
	void RecordRenderCommands(std::vector<vk::CommandBuffer> &secondaryCmds);

	static vk::AttachmentLoadOp GetLoadOpFromContent(ContentTreatment content);
	static vk::AttachmentStoreOp GetStoreOpFromContent(ContentTreatment content);

	vk::UniqueRenderPass _renderPass;
	vk::UniqueFramebuffer _framebuffer;

	CmdBufferVk _passCmds;
};

NAMESPACE_END(gr1)