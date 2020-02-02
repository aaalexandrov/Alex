#pragma once

#include "resource.h"

NAMESPACE_BEGIN(gr1)

enum class DependencyType {
	None = 0,
	Input = 1,
	Output = 2,
};

using DependencyFunc = std::function<void(Resource*, ResourceState)>;

class PassData;
class OutputPass : public Resource {
	RTTR_ENABLE(Resource)
public:
	OutputPass(Device &device) : Resource(device) {}

	virtual void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) = 0;

	virtual void Prepare(PassData *passData) = 0;
	virtual void Execute(PassData *passData) = 0;
};

class ResourceStateTransitionPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	ResourceStateTransitionPass(Device &device) : OutputPass(device) {}
		 
	virtual void Init(Resource &resource, ResourceState srcState, ResourceState dstState);

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override { ASSERT(!"This shouldn't be called"); }

	template<typename ResourceType>
	ResourceType GetResource() { return static_cast<ResourceType*>(_resource); }

public:
	// Resource isn't owned because this pass is solely intended to be created by execution queue with resources that are already owned by other passes
	Resource *_resource;
	ResourceState _srcState, _dstState;
};

class Buffer;
class CopyBufferPass : public OutputPass {
	RTTR_ENABLE(OutputPass)
public:
	CopyBufferPass(Device &device) : OutputPass(device) {}

	virtual void Init(std::shared_ptr<Buffer> const &srcBuffer, std::shared_ptr<Buffer> const &dstBuffer, uint32_t offset, uint32_t size);
	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	std::shared_ptr<Buffer> _src, _dst;
	uint32_t _offset, _size;
};

class PresentationSurface;
class Image;
class PresentPass : public OutputPass {
	RTTR_ENABLE()
public:
	PresentPass(Device &device) : OutputPass(device) {}

	virtual void Init(std::shared_ptr<PresentationSurface> const &presentSurface);
	virtual void SetImageToPresent(std::shared_ptr<Image> const &surfaceImage);

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

protected:
	std::shared_ptr<PresentationSurface> _surface;
	std::shared_ptr<Image> _surfaceImage;
};

NAMESPACE_END(gr1)

