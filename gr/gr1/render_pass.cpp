#include "render_pass.h"
#include "execution_queue.h"
#include "image.h"
#include "shader.h"
#include "buffer.h"
#include "sampler.h"
#include "render_state.h"

NAMESPACE_BEGIN(gr1)

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

	bool valid = true;
	if (bufData.IsIndex()) {
		valid &= std::find_if(_buffers.begin(), _buffers.end(), [](auto &b) { return b.IsIndex(); }) == _buffers.end();
		ASSERT(valid);
	}
	if (bufData.IsVertex()) {
		valid &= _shaders[static_cast<int>(ShaderKind::Vertex)]->HasCommonVertexAttributes(bufData.GetBufferLayout()->GetArrayElement(), nullptr);
		ASSERT(valid);
	}
	if (bufData.IsUniform()) {
		bool used = false;
		for (int i = 0; i < _shaders.size(); ++i) {
			bufData._bindings[i] = ~0ul;
			if (!_shaders[i])
				continue;
			auto info = _shaders[i]->GetUniformInfo(shaderId);
			if (!info)
				continue;
			used = true;
			bufData._bindings[i] = info->_binding;
		}
		ASSERT(used);
		valid &= used;
	}

	if (!valid)
		return -1;

	_buffers.push_back(std::move(bufData));
	return static_cast<int>(_buffers.size() - 1);
}

int RenderDrawCommand::AddSampler(std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> const &image, util::StrId shaderId)
{
	SamplerData samplerData;
	samplerData._sampler = sampler;
	samplerData._image = image;

	bool used = false;
	for (int i = 0; i < _shaders.size(); ++i) {
		samplerData._bindings[i] = ~0ul;
		if (!_shaders[i])
			continue;
		auto info = _shaders[i]->GetSamplerInfo(shaderId);
		if (!info)
			continue;
		used = true;
		samplerData._bindings[i] = info->_binding;
	}
	ASSERT(used);
	if (!used)
		return -1;

	_samplers.push_back(std::move(samplerData));
	return static_cast<int>(_samplers.size() - 1);
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

glm::uvec2 RenderPass::GetRenderAreaSize()
{
	glm::uvec2 size(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
	for (auto &attach : _attachments) {
		size = glm::min(size, glm::uvec2(attach._image->GetSize()));
	}
	return size;
}

void RenderPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	for (auto &attach : _attachments) {
		addDependencyFunc(attach._image.get(), ResourceState::RenderWrite);
	}

	for (auto &cmd : _commands) {
		cmd->GetDependencies(dependencyType, addDependencyFunc);
	}
}

void RenderPass::ClearAttachments()
{
	_attachments.clear();
}

int RenderPass::AddAttachment(ContentTreatment inputContent, ContentTreatment outputContent, glm::vec4 clearValue)
{
	ASSERT(outputContent != ContentTreatment::Clear);
	_attachments.emplace_back();

	_attachments.back()._inputContent = inputContent;
	_attachments.back()._outputContent = outputContent;
	_attachments.back()._clearValue = clearValue;

	return static_cast<int>(_attachments.size() - 1);
}

void RenderPass::SetAttachmentImage(int attachmentIndex, std::shared_ptr<Image> const &image)
{
	_attachments[attachmentIndex]._image = image;
}

void RenderPass::ClearCommands()
{
	_commands.clear();
}

void RenderPass::AddCommand(std::shared_ptr<RenderCommand> const &cmd)
{
	_commands.push_back(cmd);
}


NAMESPACE_END(gr1)

