#pragma once

#include "vk.h"
#include "../presentation_surface.h"

namespace gr {

class GraphicsVk;
class PhysicalDeviceVk;

class PresentationSurfaceVk : public PresentationSurface {
public:
  PresentationSurfaceVk(PhysicalDeviceVk *physicalDevice, PresentationSurfaceCreateData &createData);

#if defined(_WIN32)
  void InitSurfaceWin32(PresentationSurfaceCreateData *createData);
#elif defined(linux)
  void InitSurfaceXlib(PresentationSurfaceCreateData *createData);
#endif

  int32_t GetPresentQueueFamily();
  vk::SurfaceFormatKHR GetSurfaceFormat();

  vk::PhysicalDevice &GetPhysicalDevice();
  vk::Instance &GetInstance();
  GraphicsVk *GetGraphics();

  PhysicalDeviceVk *_physicalDevice;
  vk::UniqueSurfaceKHR _surface;
  vk::SurfaceFormatKHR _surfaceFormat;
};

}