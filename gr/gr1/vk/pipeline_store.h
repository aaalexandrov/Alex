#pragma once

#include "../execution_queue.h"
#include "../shader.h"
#include "../render_pass.h"
#include "../render_state.h"
#include "descriptor_set_store.h"
#include "util/utl.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class ShaderVk;
class DeviceVk;

struct PipelineLayoutVk {
	void Init(DeviceVk &deviceVk, ShaderKindsArray<ShaderVk*> &shaders);

	vk::PipelineLayout &Get() { return *_pipelineLayout; }

	ShaderKindsArray<std::weak_ptr<ShaderVk>> _shaders;
	vk::UniqueDescriptorSetLayout _descriptorSetLayout;
	vk::UniquePipelineLayout _pipelineLayout;
	DescriptorSetStore _descriptorSetStore;
};

struct PipelineVk {
	PipelineVk(std::shared_ptr<PipelineLayoutVk> const layout, vk::UniquePipeline &&pipeline) 
		: _pipelineLayout(std::move(layout)), _pipeline(std::move(pipeline)) {}

	vk::Pipeline &Get() { return *_pipeline; }

	vk::UniqueDescriptorSet AllocateDescriptorSet();

	std::shared_ptr<PipelineLayoutVk> _pipelineLayout;
	vk::UniquePipeline _pipeline;
};

struct VertexBufferLayout {
	std::shared_ptr<util::LayoutElement> _bufferLayout;
	uint32_t _binding = 0;
	bool _frequencyInstance = false;

	size_t GetHash() const;
	bool operator==(VertexBufferLayout const &other) const;
};

class RenderPassVk;
class RenderState;
struct PipelineInfo {
	ShaderKindsArray<ShaderVk*> _shaders;
	std::vector<VertexBufferLayout> _vertexBufferLayouts;
	RenderState::StateData const *_renderStateData;
	PrimitiveKind _primitiveKind;
	RenderPassVk *_renderPass;
	uint32_t _subpass;

	size_t GetHash();
	bool operator==(PipelineInfo const &other) const;
};

struct RecordedPipeline {
	RecordedPipeline(PipelineInfo *pipelineInfo) : _pipelineInfo(pipelineInfo) {}

	void CopyToOwnData();

	bool IsValid() const;

	PipelineInfo *_pipelineInfo;
	std::unique_ptr<PipelineInfo> _ownPipelineInfo;
	std::weak_ptr<RenderPassVk> _weakRenderPass;
	std::unique_ptr<RenderState::StateData> _ownStateData;

	bool operator==(RecordedPipeline const &other) const;
};

NAMESPACE_END(gr1)

NAMESPACE_BEGIN(std)
template <> 
struct hash<gr1::VertexBufferLayout> {
	size_t operator()(gr1::VertexBufferLayout const &key) const { return key.GetHash(); }
};

template <>
struct hash<gr1::RecordedPipeline> {
	size_t operator()(gr1::RecordedPipeline const &key) const { return key._pipelineInfo->GetHash(); }
};
NAMESPACE_END(std)

NAMESPACE_BEGIN(gr1)

class PipelineStore {
	RTTR_ENABLE()
public:

	void Init(DeviceVk &deviceVk) { _deviceVk = &deviceVk; }

	std::shared_ptr<PipelineVk> GetPipeline(PipelineInfo &pipelineInfo);
protected:
	// the shared_ptrs will retain the pipeline layouts forever
	using PipelineLayoutsMap = std::unordered_map<ShaderKindsArray<ShaderVk*>, std::shared_ptr<PipelineLayoutVk>>;
	using PipelinesMap = std::unordered_map<RecordedPipeline, std::weak_ptr<PipelineVk>>;

	int GetVertexLayoutIndex(std::string attribName, util::LayoutElement const *attribLayout, std::vector<VertexBufferLayout> &bufferLayouts);

	std::shared_ptr<PipelineVk> AllocatePipeline(std::shared_ptr<PipelineLayoutVk> const &pipelineLayout, PipelineInfo &pipelineInfo);

	PipelineLayoutsMap::iterator GetExistingPipelineLayout(ShaderKindsArray<ShaderVk*> &shaders);
	std::shared_ptr<PipelineLayoutVk> AddPipelineLayout(ShaderKindsArray<ShaderVk*> &shaders);

	DeviceVk *_deviceVk = nullptr;
	PipelineLayoutsMap _pipelineLayouts;
	PipelinesMap _pipelines;
};


NAMESPACE_END(gr1)