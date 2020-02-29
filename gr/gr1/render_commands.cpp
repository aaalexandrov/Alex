#include "render_commands.h"
#include "execution_queue.h"
#include "shader.h"
#include "buffer.h"
#include "image.h"
#include "sampler.h"
#include "render_state.h"
#include "render_pipeline.h"

NAMESPACE_BEGIN(gr1)

PipelineDrawCommand::ResourceData::ResourceData(PipelineResource::Kind kind)
{
	Alloc(kind);
}

PipelineDrawCommand::ResourceData::ResourceData(ResourceData const &other)
{
	Alloc(other._kind);
	Copy(other);
}

PipelineDrawCommand::ResourceData::~ResourceData()
{
	Free();
}

PipelineDrawCommand::ResourceData &PipelineDrawCommand::ResourceData::operator=(ResourceData const &other)
{
	Free();
	Alloc(other._kind);
	Copy(other);
	return *this;
}

void PipelineDrawCommand::ResourceData::Alloc(PipelineResource::Kind kind)
{
	_kind = kind;
	if (_kind == PipelineResource::Invalid)
		return;
	if (_kind == PipelineResource::Buffer) {
		BufferData *buf = new (_data) BufferData();
		ASSERT(static_cast<void*>(buf) == _data);
	} else {
		ASSERT(_kind == PipelineResource::Sampler);
		SamplerData *samp = new (_data) SamplerData();
		ASSERT(static_cast<void*>(samp) == _data);
	}
}

void PipelineDrawCommand::ResourceData::Free()
{
	if (_kind == PipelineResource::Invalid)
		return;
	if (_kind == PipelineResource::Buffer) {
		GetBufferData()->~BufferData();
	} else {
		ASSERT(_kind == PipelineResource::Sampler);
		GetSamplerData()->~SamplerData();
	}
}

void PipelineDrawCommand::ResourceData::Copy(ResourceData const &other)
{
	ASSERT(_kind == other._kind);
	if (_kind == PipelineResource::Invalid)
		return;
	if (_kind == PipelineResource::Buffer) {
		*GetBufferData() = *const_cast<ResourceData&>(other).GetBufferData();
	} else {
		ASSERT(_kind == PipelineResource::Sampler);
		*GetSamplerData() = *const_cast<ResourceData&>(other).GetSamplerData();
	}
}

void PipelineDrawCommand::Init(std::shared_ptr<RenderPipeline> const &pipeline)
{
	ASSERT(!_pipeline && !_resources.size());
	_pipeline = pipeline;
	_resources.resize(_pipeline->GetResourceInfos().size());
	for (uint32_t i = 0; i < _pipeline->GetResourceInfos().size(); ++i) {
		auto &info = _pipeline->GetResourceInfos()[i];
		_resources[i] = ResourceData(info.GetPipelineResourceKind());
	}
}

int32_t PipelineDrawCommand::SetBuffer(util::StrId bufferId, std::shared_ptr<Buffer> const &buffer, size_t offset, size_t size)
{
	int32_t ind = _pipeline->GetResourceInfoIndex(bufferId);
	if (ind >= 0 && _resources[ind].GetKind() == PipelineResource::Buffer) {
		BufferData *bufData = _resources[ind].GetBufferData();
		bufData->_buffer = buffer;
		bufData->_offset = offset;
		bufData->_size = size;
	}
	return ind;
}

int32_t PipelineDrawCommand::SetSampler(util::StrId samplerId, std::shared_ptr<Sampler> const & sampler, std::shared_ptr<Image>& image)
{
	int32_t ind = _pipeline->GetResourceInfoIndex(samplerId);
	if (ind >= 0 && _resources[ind].GetKind() == PipelineResource::Sampler) {
		SamplerData *sampData = _resources[ind].GetSamplerData();
		sampData->_sampler = sampler;
		sampData->_image = image;
	}
	return ind;
}

auto PipelineDrawCommand::GetBufferData(util::StrId bufferId) const ->BufferData const *
{
	int32_t ind = _pipeline->GetResourceInfoIndex(bufferId);
	if (ind >= 0 && _resources[ind].GetKind() == PipelineResource::Buffer) {
		BufferData *bufData = _resources[ind].GetBufferData();
		return bufData;
	}
	return nullptr;
}

auto PipelineDrawCommand::GetSamplerData(util::StrId samplerId) const ->SamplerData const *
{
	int32_t ind = _pipeline->GetResourceInfoIndex(samplerId);
	if (ind >= 0 && _resources[ind].GetKind() == PipelineResource::Sampler) {
		SamplerData *sampData = _resources[ind].GetSamplerData();
		return sampData;
	}
	return nullptr;
}

void PipelineDrawCommand::SetDrawCounts(uint32_t indexCount, uint32_t firstIndex, uint32_t instanceCount, uint32_t firstInstance, uint32_t vertexOffset)
{
	_drawCounts._indexCount = indexCount;
	_drawCounts._firstIndex = firstIndex;
	_drawCounts._instanceCount = instanceCount;
	_drawCounts._firstInstance = firstInstance;
	_drawCounts._vertexOffset = vertexOffset;
}

void PipelineDrawCommand::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	addDependencyFunc(_pipeline.get(), ResourceState::ShaderRead);
	for (ResourceData &res : _resources) {
		if (res.GetKind() == PipelineResource::Buffer) {
			BufferData *bufData = res.GetBufferData();
			if (bufData->_buffer) {
				addDependencyFunc(bufData->_buffer.get(), ResourceState::ShaderRead);
			}
		} else {
			ASSERT(res.GetKind() == PipelineResource::Sampler);
			SamplerData *sampData = res.GetSamplerData();
			if (sampData->_sampler) {
				addDependencyFunc(sampData->_sampler.get(), ResourceState::ShaderRead);
			}
			if (sampData->_image) {
				addDependencyFunc(sampData->_image.get(), ResourceState::ShaderRead);
			}
		}
	}
}

std::shared_ptr<util::LayoutElement> const &RenderDrawCommand::BufferData::GetBufferLayout() const
{
	return _overrideLayout ? _overrideLayout : _buffer->GetBufferLayout();
}

bool RenderDrawCommand::BufferData::IsVertex() const
{
	return !!(_buffer->GetUsage() & Buffer::Usage::Vertex) && GetBufferLayout()->GetArrayElement()->IsStruct();
}

bool RenderDrawCommand::BufferData::IsIndex() const
{
	return !!(_buffer->GetUsage() & Buffer::Usage::Index) && GetBufferLayout()->GetArrayElement()->IsValue();
}

bool RenderDrawCommand::BufferData::IsUniform() const
{
	return !!(_buffer->GetUsage() & Buffer::Usage::Uniform);
}

bool RenderDrawCommand::SetBinding(ShaderKindsArray<uint32_t> &bindings, ShaderKind::Enum kind, uint32_t binding)
{
	ASSERT(binding != ~0ul);
	for (int i = 0; i < kind; ++i) {
		if (bindings[i] == binding)
			return false;
	}
	bindings[kind] = binding;
	return true;
}

Shader::Parameter const *RenderDrawCommand::GetShaderParamFromBinding(ShaderKindsArray<uint32_t> &bindings, Shader::Parameter::Kind paramKind)
{
	for (int i = 0; i < bindings.size(); ++i) {
		if (bindings[i] == ~0ul)
			continue;
		Shader *shader = _shaders[i].get();
		Shader::Parameter const *param = shader->GetParamInfo(paramKind, bindings[i]);
		if (param)
			return param;
	}
	return nullptr;
}

void RenderDrawCommand::Clear()
{
	std::fill(_shaders.begin(), _shaders.end(), nullptr);
	_buffers.clear();
	_renderState.reset();
	_primitiveKind = PrimitiveKind::TriangleList;
	_drawCounts = DrawCounts();
}

int RenderDrawCommand::AddBuffer(std::shared_ptr<Buffer> const &buffer, util::StrId shaderId, size_t offset, bool frequencyInstance, std::shared_ptr<util::LayoutElement> const &overrideLayout)
{
	BufferData bufData;
	bufData._buffer = buffer;
	bufData._offset = offset;
	bufData._frequencyInstance = frequencyInstance;
	bufData._overrideLayout = overrideLayout;
	bufData._parameterId = shaderId;

	bool valid = true;
	if (bufData.IsIndex()) {
		valid &= std::find_if(_buffers.begin(), _buffers.end(), [](auto &b) { return b.IsIndex(); }) == _buffers.end();
		ASSERT(valid);
	}
	if (bufData.IsVertex()) {
		valid &= _shaders[ShaderKind::Vertex]->HasCommonVertexAttributes(bufData.GetBufferLayout()->GetArrayElement(), nullptr);
		ASSERT(valid);
	}
	if (bufData.IsUniform()) {
		bool used = false;
		for (int i = 0; i < _shaders.size(); ++i) {
			bufData._bindings[i] = ~0ul;
			if (!_shaders[i])
				continue;
			auto info = _shaders[i]->GetParamInfo(Shader::Parameter::UniformBuffer, shaderId);
			if (!info)
				continue;
			used = true;
			SetBinding(bufData._bindings, static_cast<ShaderKind::Enum>(i), info->_binding);
		}
		ASSERT(used);
		valid &= used;
	}

	if (!valid)
		return -1;

	_buffers.push_back(std::move(bufData));
	return static_cast<int>(_buffers.size() - 1);
}

int RenderDrawCommand::GetBufferIndex(util::StrId shaderId)
{
	auto it = std::find_if(_buffers.begin(), _buffers.end(), [&](auto &b) { return b._parameterId == shaderId; });
	if (it == _buffers.end())
		return -1;
	ASSERT(GetShaderParamFromBinding(it->_bindings, Shader::Parameter::UniformBuffer)->_id == shaderId);
	return static_cast<int>(it - _buffers.begin());
}

int RenderDrawCommand::AddSampler(std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> const &image, util::StrId shaderId)
{
	SamplerData samplerData;
	samplerData._sampler = sampler;
	samplerData._image = image;
	samplerData._parameterId = shaderId;

	bool used = false;
	for (int i = 0; i < _shaders.size(); ++i) {
		samplerData._bindings[i] = ~0ul;
		if (!_shaders[i])
			continue;
		auto info = _shaders[i]->GetParamInfo(Shader::Parameter::Sampler, shaderId);
		if (!info)
			continue;
		used = true;
		SetBinding(samplerData._bindings, static_cast<ShaderKind::Enum>(i), info->_binding);
	}
	ASSERT(used);
	if (!used)
		return -1;

	_samplers.push_back(std::move(samplerData));
	return static_cast<int>(_samplers.size() - 1);
}

int RenderDrawCommand::GetSamplerIndex(util::StrId shaderId)
{
	auto it = std::find_if(_samplers.begin(), _samplers.end(), [&](auto &s) { return s._parameterId == shaderId; });
	if (it == _samplers.end())
		return -1;
	ASSERT(GetShaderParamFromBinding(it->_bindings, Shader::Parameter::Sampler)->_id == shaderId);
	return static_cast<int>(it - _samplers.begin());
}

void RenderDrawCommand::SetDrawCounts(uint32_t indexCount, uint32_t firstIndex, uint32_t instanceCount, uint32_t firstInstance, uint32_t vertexOffset)
{
	_drawCounts._indexCount = indexCount;
	_drawCounts._firstIndex = firstIndex;
	_drawCounts._instanceCount = instanceCount;
	_drawCounts._firstInstance = firstInstance;
	_drawCounts._vertexOffset = vertexOffset;
}

void RenderDrawCommand::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		for (auto &shader : _shaders) {
			if (shader) {
				addDependencyFunc(shader.get(), ResourceState::ShaderRead);
			}
		}
		for (auto &bufData : _buffers) {
			addDependencyFunc(bufData._buffer.get(), ResourceState::ShaderRead);
		}
		for (auto &samplerData : _samplers) {
			//addDependencyFunc(samplerData._sampler.get(), ResourceState::ShaderRead);
			Image *image = samplerData._image.get();
			if (image)
				addDependencyFunc(image, ResourceState::ShaderRead);
		}
		addDependencyFunc(_renderState.get(), ResourceState::ShaderRead);
	}
}

NAMESPACE_END(gr1)