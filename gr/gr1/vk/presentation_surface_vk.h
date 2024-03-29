#pragma once

#include "vk.h"
#include "../presentation_surface.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;

class PresentationSurfaceVk : public PresentationSurface {
	RTTR_ENABLE(PresentationSurface)
public:
	PresentationSurfaceVk(Device &device) : PresentationSurface(device) {}

	std::vector<ColorFormat> GetSupportedSurfaceFormats() override;
	std::vector<PresentMode> GetSupportedPresentModes() override;

	void Init(PresentationSurfaceCreateData &createData) override;

	void Update(uint32_t width, uint32_t height) override;
	glm::uvec2 GetSize() override;

	std::shared_ptr<Image> const &AcquireNextImage() override;

public:
	void CreateSwapChain(uint32_t width, uint32_t height);

#if defined(_WIN32)
	void InitSurfaceWin32( PresentationSurfaceCreateData *createData);
#elif defined(__linux__)
	void InitSurfaceXlib(PresentationSurfaceCreateData *createData);
#endif

	vk::SurfaceFormatKHR GetVkSurfaceFormat();
	vk::PresentModeKHR GetVkPresentMode();

	uint32_t GetImageIndex(std::shared_ptr<Image> const &image);

	static vk::Extent2D GetExtent(vk::SurfaceCapabilitiesKHR const &surfaceCaps, uint32_t width, uint32_t height);

	vk::UniqueSurfaceKHR _surface;
	vk::UniqueSwapchainKHR _swapchain;
	vk::UniqueSemaphore _acquireSemaphore;
	std::vector<std::shared_ptr<Image>> _images;

	static util::ValueRemapper<vk::PresentModeKHR, PresentMode> s_vk2PresentMode;
};

NAMESPACE_END(gr1)
