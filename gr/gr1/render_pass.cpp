#include "render_pass.h"
#include "image.h"

NAMESPACE_BEGIN(gr1)

void RenderPass::GetDependencies(DependencyType dependencyType, std::unordered_set<Resource*> &dependencies)
{
	// Attachments are read and write dependencies
	for (auto &attach : _attachments) {
		dependencies.insert(attach._image.get());
	}

	for (auto &cmd : _commands) {
		cmd->GetDependencies(dependencyType, dependencies);
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

