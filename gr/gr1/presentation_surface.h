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

	virtual void Init(PresentationSurfaceCreateData &createData) = 0;

	virtual std::vector<ColorFormat> GetSupportedSurfaceFormats() = 0;
	virtual void SetSurfaceFormat(ColorFormat format);
	inline ColorFormat GetSurfaceFormat() { return _surfaceFormat; }

	virtual std::vector<PresentMode> GetSupportedPresentModes() = 0;
	virtual void SetPresentMode(PresentMode mode);
	inline PresentMode GetPresentMode() { return _presentMode; }

	virtual void Update(uint32_t width, uint32_t height) = 0;
	virtual glm::uvec2 GetSize() = 0;


	virtual std::shared_ptr<Image> const &AcquireNextImage() = 0;


	ColorFormat GetFirstAvailableSurfaceFormat(std::vector<ColorFormat> const &desiredFormats);
	PresentMode GetFirstAvailablePresentMode(std::vector<PresentMode> const &desiredModes);
protected:
	ColorFormat _surfaceFormat = ColorFormat::B8G8R8A8;
	PresentMode _presentMode = PresentMode::Fifo;
};

NAMESPACE_END(gr1)

