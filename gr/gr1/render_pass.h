#pragma once

#include "output_pass.h"
#include "device.h"
#include "rttr_factory.h"

NAMESPACE_BEGIN(gr1)

class Buffer;
class Image;
class Shader;
class RenderState;

class RenderCommand {
	RTTR_ENABLE()
public:
	virtual ~RenderCommand() {}

	virtual void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) = 0;
};

class RenderDrawCommand : public RenderCommand {
	RTTR_ENABLE(RenderCommand)
public:


public:
	struct BufferData {
		std::shared_ptr<Buffer> _buffer;
		int _binding;
	};

	std::vector<std::shared_ptr<Shader>> _shaders;
	std::shared_ptr<RenderState> _renderState;
	std::vector<BufferData> _buffers;
};

class RenderPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	RenderPass(Device &device) : OutputPass(device), _cmdFactory(RttrDiscriminator::Tag, device.GetResourceDiscriminatorType()) {}

	virtual std::shared_ptr<RenderCommand> CreateCommand(rttr::type cmdType) { return _cmdFactory.CreateInstanceShared<RenderCommand>(cmdType); }
	
	template<typename CmdType>
	std::shared_ptr<CmdType> CreateCommand() { return std::static_pointer_cast<CmdType>(CreateCommand(rttr::type::get<CmdType>())); }

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

	RttrFactory _cmdFactory;
	std::vector<std::shared_ptr<RenderCommand>> _commands;
};

NAMESPACE_END(gr1)

