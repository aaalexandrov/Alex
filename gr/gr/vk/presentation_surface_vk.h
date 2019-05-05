#pragma once

#include "vk.h"
#include "../presentation_surface.h"

NAMESPACE_BEGIN(gr)

class GraphicsVk;
class DeviceVk;

class PresentationSurfaceVk : public PresentationSurface {
public:
  PresentationSurfaceVk(GraphicsVk *graphics, PresentationSurfaceCreateData &createData);

  void Update(uint32_t width, uint32_t height) override;

  void InitForDevice(DeviceVk *device);
  void CreateSwapChain(uint32_t width, uint32_t height);

#if defined(_WIN32)
  void InitSurfaceWin32(GraphicsVk *graphics, PresentationSurfaceCreateData *createData);
#elif defined(linux)
  void InitSurfaceXlib(GraphicsVk *graphics, PresentationSurfaceCreateData *createData);
#endif

  static vk::Extent2D GetExtent(vk::SurfaceCapabilitiesKHR const &surfaceCaps, uint32_t width, uint32_t height);

  DeviceVk *_device = nullptr;
  vk::UniqueSurfaceKHR _surface;
  vk::SurfaceFormatKHR _surfaceFormat;
  vk::PresentModeKHR _presentMode;
  vk::UniqueSwapchainKHR _swapchain;
};

NAMESPACE_END(gr)