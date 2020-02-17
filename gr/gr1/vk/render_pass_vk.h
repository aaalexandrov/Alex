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

class RenderDrawCommandVk : public RenderDrawCommand {
	RTTR_ENABLE(RenderDrawCommand)
public:
	RenderDrawCommandVk(Device &device);

	void Clear() override;
	void SetRenderState(std::shared_ptr<RenderState> const &renderState) override;
	void SetShader(std::shared_ptr<Shader> const &shader) override;
	void RemoveShader(ShaderKind kind) override;
	int AddBuffer(std::shared_ptr<Buffer> const &buffer, int binding = 0, size_t offset = 0, bool frequencyInstance = false) override;
	void RemoveBuffer(int bufferIndex) override;	void SetPrimitiveKind(PrimitiveKind primitiveKind) override;
	void SetDrawCounts(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0, uint32_t vertexOffset = 0) override;
	int AddSampler(std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> const &image, int binding = 0) override;
	void RemoveSampler(int samplerIndex) override;
	void PrepareToRecord(CommandPrepareInfo &prepareInfo) override;
	void Record(CommandRecordInfo &recordInfo) override;

protected:
	void PreparePipeline(RenderPassVk *renderPass, uint32_t subpass);
	void PrepareDescriptorSets();

	uint32_t GetBuffersDescriptorCount();
	uint32_t GetSamplersDescriptorCount();
	void UpdateDescriptorSets();
	void SetDynamicState(CommandPrepareInfoVk &prepareInfo);

	CmdBufferVk _cmdDraw;
	std::shared_ptr<PipelineVk> _pipeline;
	uint32_t _pipelineRenderStateVersion;
	DescriptorSetVk _descriptorSet;
	bool _descriptorSetValid;
	vk::Viewport _recordedViewport{};
};

class RenderPassVk : public RenderPass, public PassVk {
	RTTR_ENABLE(RenderPass, PassVk)
public:
	RenderPassVk(Device &device);

	void ClearAttachments() override;
	int AddAttachment(ContentTreatment inputContent, ContentTreatment outputContent, glm::vec4 clearValue = glm::vec4()) override;
	void SetAttachmentImage(int attachmentIndex, std::shared_ptr<Image> const &img) override;

	void Prepare() override;
	void Execute(PassDependencyTracker &dependencies) override;

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