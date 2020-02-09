#pragma once

#include "output_pass.h"
#include "device.h"
#include "shader.h"

NAMESPACE_BEGIN(gr1)

class Buffer;
class Image;
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

enum class PrimitiveKind {
	TriangleList,
	TriangleStrip,
};

class RenderDrawCommand : public RenderCommand {
	RTTR_ENABLE(RenderCommand)
public:
	struct BufferData {
		std::shared_ptr<Buffer> _buffer;
		size_t _offset = 0;
		ShaderKindBits _shaderKinds = {};
		int _binding = 0;
		bool _frequencyInstance = false;
	};

	struct DrawCounts {
		uint32_t _indexCount = 0, _firstIndex = 0, _instanceCount = 1, _firstInstance = 0, _vertexOffset = 0;
	};

	RenderDrawCommand(Device &device) : RenderCommand(device) {}

	virtual void Clear();

	virtual void SetRenderState(std::shared_ptr<RenderState> const &renderState) { _renderState = renderState; }
	std::shared_ptr<RenderState> const &GetRenderState() { return _renderState; }

	virtual void SetShader(std::shared_ptr<Shader> const &shader) { _shaders[static_cast<int>(shader->GetShaderKind())] = shader; }
	virtual void RemoveShader(ShaderKind kind) { _shaders[static_cast<int>(kind)].reset(); }
	std::shared_ptr<Shader> const &GetShader(ShaderKind kind) { return _shaders[static_cast<int>(kind)]; }

	virtual int AddBuffer(std::shared_ptr<Buffer> const &buffer, ShaderKindBits shaderKinds = ShaderKindBits::None, int binding = 0, size_t offset = 0, bool frequencyInstance = false);
	virtual void RemoveBuffer(int bufferIndex) { _buffers.erase(_buffers.begin() + bufferIndex); }
	int GetBufferCount() { return static_cast<int>(_buffers.size()); }
	BufferData const &GetBufferData(int bufferIndex) { return _buffers[bufferIndex]; }

	virtual void SetPrimitiveKind(PrimitiveKind primitiveKind) { _primitiveKind = primitiveKind; }
	PrimitiveKind GetPrimitiveKind() { return _primitiveKind; }

	virtual void SetDrawCounts(uint32_t indexCount, uint32_t firstIndex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0, uint32_t vertexOffset = 0);
	DrawCounts const &GetDrawCounts() { return _drawCounts; }

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	ShaderKindsArray<std::shared_ptr<Shader>> _shaders;
	std::shared_ptr<RenderState> _renderState;
	std::vector<BufferData> _buffers;
	PrimitiveKind _primitiveKind = PrimitiveKind::TriangleList;
	DrawCounts _drawCounts;
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

