#pragma once

#include "render_pass.h"

NAMESPACE_BEGIN(gr1)

struct DrawCounts {
	uint32_t _indexCount = 0, _firstIndex = 0, _instanceCount = 1, _firstInstance = 0, _vertexOffset = 0;
};

struct BufferData {
	std::shared_ptr<Buffer> _buffer;
	size_t _offset, _size;
};

struct SamplerData {
	std::shared_ptr<Sampler> _sampler;
	std::shared_ptr<Image> _image;
};

class RenderPipeline;
class PipelineDrawCommand : public RenderCommand {
	RTTR_ENABLE(RenderCommand)
public:

	struct ResourceData {
		ResourceData(ResourceData const &other);
		ResourceData(PipelineResource::Kind kind = PipelineResource::Invalid);
		~ResourceData();

		PipelineResource::Kind GetKind() const { return _kind; }
		BufferData *GetBufferData() const { ASSERT(_kind == PipelineResource::Buffer); return (BufferData *)_data; }
		SamplerData *GetSamplerData() const { ASSERT(_kind == PipelineResource::Sampler); return (SamplerData *)_data; }

		ResourceData &operator=(ResourceData const &other);
	private:
		void Alloc(PipelineResource::Kind kind);
		void Free();
		void Copy(ResourceData const &other);

		PipelineResource::Kind _kind;
		void* _data[(std::max(sizeof(BufferData), sizeof(SamplerData)) + sizeof(void*) - 1) / sizeof(void*)];
	};
	static_assert(alignof(BufferData) == alignof(void*));
	static_assert(alignof(SamplerData) == alignof(void*));

	PipelineDrawCommand(Device &device) : RenderCommand(device) {}

	virtual void Init(std::shared_ptr<RenderPipeline> const &pipeline);

	std::shared_ptr<RenderPipeline> const &GetPipeline() const { return _pipeline; }

	virtual int32_t SetBuffer(util::StrId bufferId, std::shared_ptr<Buffer> const &buffer, size_t offset = 0, size_t size = ~0ull);
	virtual int32_t SetSampler(util::StrId samplerId, std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> &image);

	virtual BufferData const *GetBufferData(util::StrId bufferId) const;
	virtual SamplerData const *GetSamplerData(util::StrId samplerId) const;

	virtual void SetDrawCounts(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0, uint32_t vertexOffset = 0);
	DrawCounts const &GetDrawCounts() { return _drawCounts; }

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	std::shared_ptr<RenderPipeline> _pipeline;
	std::vector<ResourceData> _resources;
	DrawCounts _drawCounts;
};

class RenderDrawCommand : public RenderCommand {
	RTTR_ENABLE(RenderCommand)
public:
	struct BufferData {
		std::shared_ptr<Buffer> _buffer;
		size_t _offset = 0;
		util::StrId _parameterId;
		ShaderKindsArray<uint32_t> _bindings;
		bool _frequencyInstance = false;
		std::shared_ptr<util::LayoutElement> _overrideLayout;

		std::shared_ptr<util::LayoutElement> const &GetBufferLayout() const;
		bool IsVertex() const;
		bool IsIndex() const;
		bool IsUniform() const;
	};

	struct SamplerData {
		std::shared_ptr<Sampler> _sampler;
		std::shared_ptr<Image> _image;
		ShaderKindsArray<uint32_t> _bindings;
		util::StrId _parameterId;
	};

	RenderDrawCommand(Device &device) : RenderCommand(device) {}

	virtual void Clear();

	virtual void SetRenderState(std::shared_ptr<RenderState> const &renderState) { _renderState = renderState; }
	std::shared_ptr<RenderState> const &GetRenderState() { return _renderState; }

	virtual void SetShader(std::shared_ptr<Shader> const &shader) { ASSERT(!_shaders[shader->GetShaderKind()]); _shaders[shader->GetShaderKind()] = shader; }
	std::shared_ptr<Shader> const &GetShader(ShaderKind::Enum kind) { return _shaders[kind]; }

	virtual int AddBuffer(std::shared_ptr<Buffer> const &buffer, util::StrId shaderId = util::StrId(), size_t offset = 0, bool frequencyInstance = false, std::shared_ptr<util::LayoutElement> const &overrideLayout = std::shared_ptr<util::LayoutElement>());
	virtual void RemoveBuffer(int bufferIndex) { _buffers.erase(_buffers.begin() + bufferIndex); }
	int GetBufferCount() { return static_cast<int>(_buffers.size()); }
	BufferData const &GetBufferData(int bufferIndex) { return _buffers[bufferIndex]; }
	int GetBufferIndex(util::StrId shaderId);

	virtual int AddSampler(std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> const &image, util::StrId shaderId);
	virtual void RemoveSampler(int samplerIndex) { _samplers.erase(_samplers.begin() + samplerIndex); }
	int GetSamplerCount() { return static_cast<int>(_samplers.size()); }
	SamplerData const &GetSamplerData(int samplerIndex) { return _samplers[samplerIndex]; }
	int GetSamplerIndex(util::StrId shaderId);

	virtual void SetPrimitiveKind(PrimitiveKind primitiveKind) { _primitiveKind = primitiveKind; }
	PrimitiveKind GetPrimitiveKind() { return _primitiveKind; }

	virtual void SetDrawCounts(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0, uint32_t vertexOffset = 0);
	DrawCounts const &GetDrawCounts() { return _drawCounts; }

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	static bool SetBinding(ShaderKindsArray<uint32_t> &bindings, ShaderKind::Enum kind, uint32_t binding);
	Shader::Parameter const *GetShaderParamFromBinding(ShaderKindsArray<uint32_t> &bindings, Shader::Parameter::Kind paramKind);

	ShaderKindsArray<std::shared_ptr<Shader>> _shaders;
	std::shared_ptr<RenderState> _renderState;
	std::vector<BufferData> _buffers;
	std::vector<SamplerData> _samplers;
	PrimitiveKind _primitiveKind = PrimitiveKind::TriangleList;
	DrawCounts _drawCounts;
};

NAMESPACE_END(gr1)