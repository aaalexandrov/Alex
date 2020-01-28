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
	rttr::type GetStateTransitionPassType() override { throw "Implement it!"; }

	void Update(uint32_t width, uint32_t height) override;
	glm::uvec2 GetSize() override;

	std::shared_ptr<Image> const &AcquireNextImage() override { return AcquireNextImage(nullptr, nullptr); }
	std::shared_ptr<Image> const &AcquireNextImage(vk::Semaphore signalSemaphore, vk::Fence fence);
	std::shared_ptr<Image> const &GetCurrentImage() override;

public:
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
