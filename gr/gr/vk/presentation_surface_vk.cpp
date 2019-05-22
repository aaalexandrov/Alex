#include "presentation_surface_vk.h"
#include "graphics_vk.h"
#include "physical_device_vk.h"
#include "device_vk.h"
#include "util/mathutl.h"

#if defined(_WIN32)
#include "../win32/presentation_surface_create_data_win32.h"
#elif defined(linux)
#error Unimplemented
#else
#error Unsupported platform
#endif

NAMESPACE_BEGIN(gr)

PresentationSurfaceVk::PresentationSurfaceVk(GraphicsVk *graphics, PresentationSurfaceCreateData &createData)
  : PresentationSurface{ createData }
{
#if defined(_WIN32)
  InitSurfaceWin32(graphics, &createData);
#elif defined(linux)
  InitSurfaceXlib(graphics, &createData);
#endif
}

util::TypeInfo * PresentationSurfaceVk::GetType()
{
  return util::TypeInfo::Get<PresentationSurfaceVk>();
}

#if defined(_WIN32)

void PresentationSurfaceVk::InitSurfaceWin32(GraphicsVk *graphics, PresentationSurfaceCreateData *createData)
{
  auto createWin32 = static_cast<PresentationSurfaceCreateDataWin32*>(createData);
  vk::Win32SurfaceCreateInfoKHR surfaceInfo(vk::Win32SurfaceCreateFlagsKHR(), createWin32->_hInstance, createWin32->_hWnd);
  _surface = graphics->_instance->createWin32SurfaceKHRUnique(surfaceInfo, graphics->AllocationCallbacks());
}

#elif defined(linux)

void PresentationSurfaceVk::InitSurfaceXlib(GraphicsVk *graphics, PresentationSurfaceCreateData *createData)
{
#error Unimplemented
}

#endif

void PresentationSurfaceVk::Update(uint32_t width, uint32_t height)
{
  CreateSwapChain(width, height);
}

void PresentationSurfaceVk::InitForDevice(DeviceVk *device)
{
  _device = device;
  _surfaceFormat = _device->_physicalDevice->GetSurfaceFormat(this);
  _presentMode = _device->_physicalDevice->GetSurfacePresentMode(this);
}

void PresentationSurfaceVk::CreateSwapChain(uint32_t width, uint32_t height)
{
  vk::SurfaceCapabilitiesKHR surfaceCaps = _device->GetPhysicalDevice().getSurfaceCapabilitiesKHR(*_surface);
  vk::Extent2D extent = GetExtent(surfaceCaps, width, height);
  uint32_t imageCount = surfaceCaps.minImageCount + 1;
  if (surfaceCaps.maxImageCount > 0)
    imageCount = std::min(imageCount, surfaceCaps.maxImageCount);

  vk::SwapchainCreateInfoKHR chainInfo;
  chainInfo
    .setSurface(*_surface)
    .setMinImageCount(imageCount)
    .setImageFormat(_surfaceFormat.format)
    .setImageColorSpace(_surfaceFormat.colorSpace)
    .setImageExtent(extent)
    .setImageArrayLayers(1)
    .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
    .setImageSharingMode(vk::SharingMode::eExclusive)
    .setPreTransform(
      (surfaceCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) 
      ? vk::SurfaceTransformFlagBitsKHR::eIdentity 
      : surfaceCaps.currentTransform)
    .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
    .setPresentMode(_presentMode)
    .setClipped(true);

  _swapchain = _device->_device->createSwapchainKHRUnique(chainInfo, _device->AllocationCallbacks());

  std::vector<vk::Image> chainImages = _device->_device->getSwapchainImagesKHR(*_swapchain);
}

vk::Extent2D PresentationSurfaceVk::GetExtent(vk::SurfaceCapabilitiesKHR const &surfaceCaps, uint32_t width, uint32_t height)
{
  if (surfaceCaps.currentExtent != std::numeric_limits<uint32_t>::max())
    return surfaceCaps.currentExtent;

  vk::Extent2D ext { 
    util::Clamp(surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width, width), 
    util::Clamp(surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height, height)
  };

  return ext;
}

NAMESPACE_END(gr)