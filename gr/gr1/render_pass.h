#pragma once

#include "output_pass.h"
#include "device.h"
#include "shader.h"

NAMESPACE_BEGIN(gr1)

class Buffer;
class Image;
class Sampler;
class RenderState;
class RenderPass;

class CommandPrepareInfo {
	RTTR_ENABLE()
public:
	CommandPrepareInfo(RenderPass *renderPass) : _renderPass(renderPass) {}

	RenderPass *_renderPass;
};

class CommandRecordInfo {
	RTTR_ENABLE()
public:
};

class RenderCommand : public ResourceBase {
	RTTR_ENABLE(ResourceBase)
public:
	RenderCommand(Device &device) : ResourceBase(device) {}

	virtual void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) = 0;

	virtual void PrepareToRecord(CommandPrepareInfo &prepareInfo) = 0;
	virtual void Record(CommandRecordInfo &recordInfo) = 0;
};

class RenderPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	RenderPass(Device &device) : OutputPass(device) {}

	virtual glm::uvec2 GetRenderAreaSize();

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

	virtual void ClearAttachments();
	virtual int AddAttachment(ContentTreatment inputContent, ContentTreatment outputContent, glm::vec4 clearValue = glm::vec4());
	virtual void SetAttachmentImage(int attachmentIndex, std::shared_ptr<Image> const &img);

	virtual void ClearCommands();
	virtual void AddCommand(std::shared_ptr<RenderCommand> const &cmd);

protected:
	struct AttachmentData {
		std::shared_ptr<Image> _image;
		ContentTreatment _inputContent, _outputContent;
		glm::vec4 _clearValue;

		template<typename ImgType>
		ImgType *GetImage() { return static_cast<ImgType*>(_image.get()); }
	};

	std::vector<AttachmentData> _attachments;

	std::vector<std::shared_ptr<RenderCommand>> _commands;
};

NAMESPACE_END(gr1)

