#pragma once

#include "graphics_vk.h"
#include <functional>
#include <vector>

NAMESPACE_BEGIN(gr)

class PresentationSurfaceVk;

enum class QueueRole {
  First,
  Graphics = First,
  Compute,
  Transfer,
  SparseOp,
  Present,
  Invalid,
  Count = Invalid,
};

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

  int32_t &QueueFamilyIndex(QueueRole role) { return _queueFamilyIndices[static_cast<int>(role)]; }

  GraphicsVk *_graphics;
  vk::PhysicalDevice _physicalDevice;
  
  vk::PhysicalDeviceProperties _properties;
  vk::PhysicalDeviceFeatures _features;
  vk::PhysicalDeviceMemoryProperties _memoryProperties;

  std::vector<vk::QueueFamilyProperties> _queueFamilies;

  std::array<int32_t, static_cast<int>(QueueRole::Count)> _queueFamilyIndices;

  vk::Format _depthFormat;

  static std::vector<vk::Format> _depthFormats;
};

NAMESPACE_END(gr)