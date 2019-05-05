#pragma once

#include "graphics_vk.h"
#include <functional>
#include <vector>

NAMESPACE_BEGIN(gr)

class PresentationSurfaceVk;

class PhysicalDeviceVk {
public:
  PhysicalDeviceVk(GraphicsVk *graphics, vk::PhysicalDevice physicalDevice, PresentationSurfaceVk *initialSurface);

  int32_t GetSuitableQueueFamily(vk::QueueFlags flags, std::function<bool(int32_t)> predicate = [](auto i) {return true;});
  int32_t GetPresentQueueFamily(PresentationSurfaceVk *surface);

  vk::SurfaceFormatKHR GetSurfaceFormat(PresentationSurfaceVk *surface);
  vk::PresentModeKHR GetSurfacePresentMode(PresentationSurfaceVk *surface);

  vk::Format GetDepthFormat();

  static bool IsDepthFormat(vk::Format format);

  uint32_t GetMemoryTypeIndex(uint32_t validMemoryTypes, vk::MemoryPropertyFlags memoryFlags);

  GraphicsVk *_graphics;
  vk::PhysicalDevice _physicalDevice;
  
  vk::PhysicalDeviceProperties _properties;
  vk::PhysicalDeviceFeatures _features;
  vk::PhysicalDeviceMemoryProperties _memoryProperties;

  std::vector<vk::QueueFamilyProperties> _queueFamilies;

  int32_t _graphicsQueueFamily;
  int32_t _computeQueueFamily;
  int32_t _transferQueueFamily;
  int32_t _sparseOpQueueFamily;
  int32_t _presentQueueFamily;

  vk::Format _depthFormat;

  static std::vector<vk::Format> _depthFormats;
};

NAMESPACE_END(gr)