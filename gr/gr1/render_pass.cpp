#include "render_pass.h"
#include "execution_queue.h"
#include "image.h"

NAMESPACE_BEGIN(gr1)

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
	if (dependencyType == DependencyType::Input) {
		for (auto &attach : _attachments) {
			addDependencyFunc(attach._image.get(), ResourceState::RenderWrite);
		}
	}

	for (auto &cmd : _commands) {
		cmd->GetDependencies(dependencyType, addDependencyFunc);
	}
}

void RenderPass::ClearAttachments()
{
	_attachments.clear();
}

void RenderPass::AddAttachment(ContentTreatment inputContent, std::shared_ptr<Image> const &img, ContentTreatment outputContent, glm::vec4 clearValue)
{
	ASSERT(outputContent != ContentTreatment::Clear);
	AttachmentData attach;
	attach._image = img;
	attach._inputContent = inputContent;
	attach._outputContent = outputContent;
	attach._clearValue = clearValue;
	_attachments.push_back(std::move(attach));
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

