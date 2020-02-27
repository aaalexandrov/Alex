#pragma once

#include "resource.h"

NAMESPACE_BEGIN(gr1)

struct PassDependencyTracker;
class OutputPass : public Resource {
	RTTR_ENABLE(Resource)
public:
	OutputPass(Device &device) : Resource(device) {}

	virtual void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) = 0;

	virtual void Prepare() = 0;
	virtual void Execute(PassDependencyTracker &dependencies) = 0;
};

class ResourceStateTransitionPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	ResourceStateTransitionPass(Device &device) : OutputPass(device) {}
		 
	virtual void Init(std::shared_ptr<Resource> const &resource, ResourceState srcState, ResourceState dstState);

	template<typename ResourceType>
	ResourceType* GetResource() { return static_cast<ResourceType*>(_resource.get()); }

public:
	std::shared_ptr<Resource> _resource;
	ResourceState _srcState, _dstState;
};

class Buffer;
class BufferCopyPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	BufferCopyPass(Device &device) : OutputPass(device) {}

	virtual void Init(std::shared_ptr<Buffer> const &srcBuffer, std::shared_ptr<Buffer> const &dstBuffer, uint32_t size = ~0, uint32_t srcOffset = 0, uint32_t dstOffset = 0);
	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	std::shared_ptr<Buffer> _src, _dst;
	uint32_t _srcOffset, _dstOffset, _size;
};

class Image;
class ImageBufferCopyPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	enum class CopyDirection {
		BufferToImage,
		ImageToBuffer,
	};

	ImageBufferCopyPass(Device &device) : OutputPass(device) {}

	virtual void Init(
		std::shared_ptr<Buffer> const &buffer, 
		std::shared_ptr<Image> const &image, 
		CopyDirection direction = CopyDirection::BufferToImage,
		glm::uvec4 bufferOffset = glm::zero<glm::uvec4>(), 
		glm::uvec4 bufferSize = glm::zero<glm::uvec4>(),
		glm::uvec4 imageOffset = glm::zero<glm::uvec4>(), 
		glm::uvec4 size = glm::zero<glm::uvec4>(),
		uint32_t imageMipLevel = 0);
	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	ResourceState GetBufferState() { return _direction == CopyDirection::BufferToImage ? ResourceState::TransferRead : ResourceState::TransferWrite; }
	ResourceState GetImageState() { return _direction == CopyDirection::BufferToImage ? ResourceState::TransferWrite : ResourceState::TransferRead; }

	CopyDirection _direction;
	std::shared_ptr<Buffer> _buffer;
	ImageData _bufferImageData;
	std::shared_ptr<Image> _image;
	glm::uvec4 _bufferOffset;
	glm::uvec4 _imageOffset;
	glm::uvec4 _size;
	uint32_t _imageMipLevel;
};

class PresentationSurface;
class PresentPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	PresentPass(Device &device) : OutputPass(device) {}

	virtual void Init(std::shared_ptr<PresentationSurface> const &presentSurface);
	virtual void SetImageToPresent(std::shared_ptr<Image> const &surfaceImage);

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	std::shared_ptr<PresentationSurface> _surface;
	std::shared_ptr<Image> _surfaceImage;
};

class FinalPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	FinalPass(Device &device) : OutputPass(device) {}

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override {}

	virtual bool IsFinished() = 0;
	virtual void WaitToFinish() = 0;
};

NAMESPACE_END(gr1)

