#include "presentation_surface_vk.h"
#include "physical_device_vk.h"

#if defined(_WIN32)
#include "../win32/presentation_surface_create_data_win32.h"
#elif defined(linux)
#error Unimplemented
#else
#error Unsupported platform
#endif

namespace gr {

PresentationSurfaceVk::PresentationSurfaceVk(PhysicalDeviceVk *physicalDevice, CreateData &createData)
  : _physicalDevice(physicalDevice), PresentationSurface(createData)
{
#if defined(_WIN32)
  InitSurfaceWin32(&createData);
#elif defined(linux)
  InitSurfaceXlib(&createData);
#endif
  _presentQueueFamily = GetPresentQueueFamily();
  if (_presentQueueFamily < 0)
    throw GraphicsException("Present queue family for surface not found", VK_ERROR_FEATURE_NOT_PRESENT);
  _surfaceFormat = GetSurfaceFormat();
}

vk::PhysicalDevice &PresentationSurfaceVk::GetPhysicalDevice()
{
  return _physicalDevice->_physicalDevice;
}

vk::Instance &PresentationSurfaceVk::GetInstance()
{
  return *GetGraphics()->_instance;
}

GraphicsVk *PresentationSurfaceVk::GetGraphics()
{
  return _physicalDevice->_graphics;
}

#if defined(_WIN32)

void PresentationSurfaceVk::InitSurfaceWin32(CreateData *createData)
{
  auto createWin32 = static_cast<PresentationSurfaceCreateDataWin32*>(createData);
  vk::Win32SurfaceCreateInfoKHR surfaceInfo(vk::Win32SurfaceCreateFlagsKHR(), createWin32->_hInstance, createWin32->_hWnd);
  _surface = GetInstance().createWin32SurfaceKHRUnique(surfaceInfo, GetGraphics()->AllocationCallbacks());
}

#elif defined(linux)

void PresentationSurfaceVk::InitSurfaceXlib(CreateData *createData)
{
#error Unimplemented
}

#endif

int32_t PresentationSurfaceVk::GetPresentQueueFamily()
{
  int32_t queueFamily = _physicalDevice->GetSuitableQueueFamily(vk::QueueFlags(), [this](int32_t q) { 
    return GetPhysicalDevice().getSurfaceSupportKHR(q, *_surface);
  });
  return queueFamily;
}

vk::SurfaceFormatKHR PresentationSurfaceVk::GetSurfaceFormat()
{
  std::vector<vk::SurfaceFormatKHR> formats = GetPhysicalDevice().getSurfaceFormatsKHR(*_surface);
  auto format = formats[0];
  if (format.format == vk::Format::eUndefined)
    format.format = vk::Format::eR8G8B8A8Unorm;
  return format;
}

}