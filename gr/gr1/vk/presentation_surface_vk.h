#pragma once

#include "vk.h"
#include "../presentation_surface.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;

class PresentationSurfaceVk : public PresentationSurface {
	RTTR_ENABLE(PresentationSurface)
public:
	PresentationSurfaceVk(Device &device) : PresentationSurface(device) {}

	void Init(PresentationSurfaceCreateData &createData) override;
	inline bool IsValid() override { return static_cast<bool>(_surface); }

	void Update(uint32_t width, uint32_t height) override;
	glm::uvec2 GetSize() override;

	Image *AcquireNextImage() override;
	Image *GetCurrentImage() override;
protected:
	void CreateSwapChain(uint32_t width, uint32_t height);

#if defined(_WIN32)
	void InitSurfaceWin32( PresentationSurfaceCreateData *createData);
#elif defined(linux)
	void InitSurfaceXlib(PresentationSurfaceCreateData *createData);
#endif

	vk::SurfaceFormatKHR GetSurfaceFormat();
	vk::PresentModeKHR GetPresentMode();

	static vk::Extent2D GetExtent(vk::SurfaceCapabilitiesKHR const &surfaceCaps, uint32_t width, uint32_t height);

	vk::UniqueSurfaceKHR _surface;
	vk::SurfaceFormatKHR _surfaceFormat;
	vk::PresentModeKHR _presentMode;
	vk::UniqueSwapchainKHR _swapchain;
	uint32_t _currentImageIndex;
	std::vector<std::shared_ptr<Image>> _images;
};

NAMESPACE_END(gr1)
