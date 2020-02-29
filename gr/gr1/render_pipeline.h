#pragma once

#include "resource.h"
#include "shader.h"

NAMESPACE_BEGIN(gr1)

struct VertexBufferLayout {
	std::shared_ptr<util::LayoutElement> _bufferLayout;
	util::StrId _bufferId;
	bool _frequencyInstance = false;

	size_t GetHash() const;
	bool operator==(VertexBufferLayout const &other) const;
};

class Shader;
class RenderState;
class RenderPass;
class RenderPipeline : public Resource {
	RTTR_ENABLE(Resource)
public:
	struct ResourceInfo {
		util::StrId _id;
		Shader::Parameter::Kind _kind;
		ShaderKindsArray<uint32_t> _indexInShader = {~0u, ~0u};
		ShaderKindsArray<uint32_t> _binding = {~0u, ~0u};

		PipelineResource::Kind GetPipelineResourceKind() const { return RenderPipeline::GetPipelineResourceKind(_kind); }
		void AddUniqueBinding(uint32_t binding);
	};

	RenderPipeline(Device &device) : Resource(device) {}

	virtual void InitParamShaders(ShaderKindsArray<std::shared_ptr<Shader>> const &shaders, PrimitiveKind primitiveKind = PrimitiveKind::TriangleList);
	virtual void InitParamVertexBufferLayout(util::StrId bufferId, std::shared_ptr<util::LayoutElement> const &bufferLayout, bool frequencyInstance = false);
	virtual void Init();

	ShaderKindsArray<std::shared_ptr<Shader>> const &GetShaders() const { return _shaders; }
	PrimitiveKind GetPrimitiveKind() const { return _primitiveKind; };
	std::vector<VertexBufferLayout> const &GetVertexBufferLayouts() const { return _vertexBufferLayouts; }

	virtual void SetRenderState(std::shared_ptr<RenderState> const &renderState);
	std::shared_ptr<RenderState> const &GetRenderState() const { return _renderState; }

	virtual void SetRenderPass(std::weak_ptr<RenderPass> const &renderPass);
	std::weak_ptr<RenderPass> GetRenderPass() const { return _renderPass; }

	std::vector<ResourceInfo> const &GetResourceInfos() const { return _resourceInfos; }
	int32_t GetResourceInfoIndex(util::StrId resourceId) const { return util::FindOrDefault(_id2Info, resourceId, -1); }

	std::vector<int32_t> const &GetVertexBufferIndices() const { return _vertexBufferIndices; };
	int32_t GetIndexBufferIndex() const { return _indexBufferIndex; }

	static PipelineResource::Kind GetPipelineResourceKind(Shader::Parameter::Kind kind);

	ResourceState UpdateResourceStateForExecute() override;
protected:
	void InitResourceInfos();

	void AddResourceInfo(ResourceInfo &info);

	ShaderKindsArray<std::shared_ptr<Shader>> _shaders;
	std::vector<VertexBufferLayout> _vertexBufferLayouts;
	std::vector<int32_t> _vertexBufferIndices;
	int32_t _indexBufferIndex;
	std::shared_ptr<RenderState> _renderState;
	PrimitiveKind _primitiveKind;
	std::weak_ptr<RenderPass> _renderPass;
	uint32_t _renderStateDataVersion = -1;

	std::vector<ResourceInfo> _resourceInfos;
	std::unordered_map<util::StrId, int32_t> _id2Info;
};

NAMESPACE_END(gr1)