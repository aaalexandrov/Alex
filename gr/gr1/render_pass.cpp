#include "render_pass.h"
#include "execution_queue.h"
#include "image.h"
#include "shader.h"
#include "buffer.h"
#include "render_state.h"

NAMESPACE_BEGIN(gr1)

void RenderDrawCommand::Clear()
{
	std::fill(_shaders.begin(), _shaders.end(), nullptr);
	_buffers.clear();
	_renderState.reset();
}

int RenderDrawCommand::AddBuffer(std::shared_ptr<Buffer> const &buffer, ShaderKindBits shaderKinds, int binding, size_t offset, bool frequencyInstance)
{ 
	_buffers.push_back(BufferData{ buffer, shaderKinds, offset, binding, frequencyInstance });
	return static_cast<int>(_buffers.size() - 1);
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
		addDependencyFunc(_renderState.get(), ResourceState::ShaderRead);
	}
}

glm::uvec2 RenderPass::GetRenderAreaSize()
{
	glm::uvec2 size(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());
	for (auto &attach : _attachments) {
		size = util::VecMin(size, glm::uvec2(attach._image->GetSize()));
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

