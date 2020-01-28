#pragma once

#include "resource.h"

NAMESPACE_BEGIN(gr1)

struct PresentationSurfaceCreateData {
	RTTR_ENABLE()
public:
};

class Image;

class PresentationSurface : public Resource {
	RTTR_ENABLE(Resource)
public:
	PresentationSurface(Device &device) : Resource(device) {}

  virtual void Init(PresentationSurfaceCreateData &createData) = 0;

  virtual void Update(uint32_t width, uint32_t height) = 0;
	virtual glm::uvec2 GetSize() = 0;

	virtual std::shared_ptr<Image> const &AcquireNextImage() = 0;
	virtual std::shared_ptr<Image> const &GetCurrentImage() = 0;
};

NAMESPACE_END(gr1)

