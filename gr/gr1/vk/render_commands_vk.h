#pragma once

#include "../render_commands.h"
#include "render_pass_vk.h"

NAMESPACE_BEGIN(gr1)

struct DynamicState {
	vk::Viewport _viewport{};

	bool operator==(DynamicState const &other)
	{
		return _viewport == other._viewport;
	}
};

class PipelineDrawCommandVk : public PipelineDrawCommand {
	RTTR_ENABLE(PipelineDrawCommand)
public:
	PipelineDrawCommandVk(Device &device);

	void PrepareToRecord(CommandPrepareInfo &prepareInfo) override;
	void Record(CommandRecordInfo &recordInfo) override;

public:
	void UpdateDescriptorSets();
	DynamicState GetDynamicState(CommandPrepareInfoVk &prepareInfo);
	void RecordDynamicState(DynamicState const &dynState);

	CmdBufferVk _cmdDraw;
	DescriptorSetVk _descriptorSet;
	bool _descriptorSetValid;
	DynamicState _recordedDynState;
};

class RenderDrawCommandVk : public RenderDrawCommand {
	RTTR_ENABLE(RenderDrawCommand)
public:
	RenderDrawCommandVk(Device &device);

	void Clear() override;
	void SetRenderState(std::shared_ptr<RenderState> const &renderState) override;
	void SetShader(std::shared_ptr<Shader> const &shader) override;
	int AddBuffer(std::shared_ptr<Buffer> const &buffer, util::StrId shaderId = util::StrId(), size_t offset = 0, bool frequencyInstance = false, std::shared_ptr<util::LayoutElement> const &overrideLayout = std::shared_ptr<util::LayoutElement>()) override;
	void RemoveBuffer(int bufferIndex) override;	void SetPrimitiveKind(PrimitiveKind primitiveKind) override;
	void SetDrawCounts(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0, uint32_t vertexOffset = 0) override;
	int AddSampler(std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> const &image, util::StrId shaderId) override;
	void RemoveSampler(int samplerIndex) override;
	void PrepareToRecord(CommandPrepareInfo &prepareInfo) override;
	void Record(CommandRecordInfo &recordInfo) override;

protected:
	void PreparePipeline(RenderPassVk *renderPass, uint32_t subpass);
	void PrepareDescriptorSets();

	uint32_t GetMaxBuffersDescriptorCount();
	uint32_t GetMaxSamplersDescriptorCount();
	void UpdateDescriptorSets();
	void SetDynamicState(CommandPrepareInfoVk &prepareInfo);

	CmdBufferVk _cmdDraw;
	std::shared_ptr<PipelineVk> _pipeline;
	uint32_t _pipelineRenderStateVersion;
	DescriptorSetVk _descriptorSet;
	bool _descriptorSetValid;
	vk::Viewport _recordedViewport{};
};

NAMESPACE_END(gr1)