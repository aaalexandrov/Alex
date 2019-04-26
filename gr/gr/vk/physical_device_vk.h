#pragma once

#include "graphics_vk.h"
#include <functional>

namespace gr {

class PhysicalDeviceVk {
public:
  PhysicalDeviceVk(GraphicsVk *graphics, vk::PhysicalDevice physicalDevice);

  int32_t GetSuitableQueueFamily(vk::QueueFlags flags, std::function<bool(int32_t)> predicate = [](auto i) {return true;});
  vk::Format GetDepthFormat();

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

  vk::Format _depthFormat;
};

}