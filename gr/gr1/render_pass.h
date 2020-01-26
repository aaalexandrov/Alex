#pragma once

#include "output_pass.h"
#include "device.h"
#include "rttr_factory.h"

NAMESPACE_BEGIN(gr1)

class Image;

class RenderCommand {
	RTTR_ENABLE()
public:
	virtual ~RenderCommand() {}

	virtual void GetDependencies(DependencyType dependencyType, std::unordered_set<Resource*> &dependencies) = 0;
};

class RenderPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	RenderPass(Device &device) : OutputPass(device), _cmdFactory(RttrDiscriminator::Tag, device.GetResourceDiscriminatorType()) {}

	virtual std::shared_ptr<RenderCommand> CreateCommand(rttr::type cmdType) { return _cmdFactory.CreateInstanceShared<RenderCommand>(cmdType); }
	
	template<typename CmdType>
	std::shared_ptr<CmdType> CreateCommand() { return std::static_pointer_cast<CmdType>(CreateCommand(rttr::type::get<CmdType>())); }

	inline bool IsValid() override { return _attachments.size(); }

	void GetDependencies(DependencyType dependencyType, std::unordered_set<Resource*> &dependencies) override;

	virtual void ClearAttachments();
	virtual void AddAttachment(ContentTreatment inputContent, std::shared_ptr<Image> const &img, ContentTreatment outputContent, glm::vec4 clearValue = glm::vec4());

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

	RttrFactory _cmdFactory;
	std::vector<std::shared_ptr<RenderCommand>> _commands;
};

NAMESPACE_END(gr1)

