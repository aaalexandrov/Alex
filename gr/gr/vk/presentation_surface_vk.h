#pragma once

#include "vk.h"
#include "../presentation_surface.h"

namespace gr {

class GraphicsVk;
class PhysicalDeviceVk;

class PresentationSurfaceVk : public PresentationSurface {
public:
  PresentationSurfaceVk(PhysicalDeviceVk *physicalDevice, CreateData &createData);

#if defined(_WIN32)
  void InitSurfaceWin32(CreateData *createData);
#elif defined(linux)
  void InitSurfaceXlib(CreateData *createData);
#endif

  int32_t GetPresentQueueFamily();
  vk::SurfaceFormatKHR GetSurfaceFormat();

  vk::PhysicalDevice &GetPhysicalDevice();
  vk::Instance &GetInstance();
  GraphicsVk *GetGraphics();

  PhysicalDeviceVk *_physicalDevice;
  vk::UniqueSurfaceKHR _surface;
  int32_t _presentQueueFamily;
  vk::SurfaceFormatKHR _surfaceFormat;
};

}