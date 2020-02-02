#pragma once

#include "resource.h"

NAMESPACE_BEGIN(gr1)

enum class PresentMode {
	Immediate,
	Mailbox,
	Fifo,
	FifoRelaxed,
	Invalid,
};

struct PresentationSurfaceCreateData {
	RTTR_ENABLE()
public:
};

class Image;

class PresentationSurface : public Resource {
	RTTR_ENABLE(Resource)
public:
	PresentationSurface(Device &device) : Resource(device) {}

	virtual void Init(PresentationSurfaceCreateData &createData, PresentMode presentMode);

  virtual void Update(uint32_t width, uint32_t height) = 0;
	virtual glm::uvec2 GetSize() = 0;

	inline PresentMode GetPresentMode() { return _presentMode; }

	virtual std::shared_ptr<Image> const &AcquireNextImage() = 0;

protected:
	PresentMode _presentMode;
};

NAMESPACE_END(gr1)

