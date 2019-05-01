#pragma once

#include "vk.h"
#include "queue_vk.h"
#include <mutex>

namespace gr {

class GraphicsVk;
class PhysicalDeviceVk;

class DeviceVk {
public:
  DeviceVk(PhysicalDeviceVk *physicalDevice);

  void InitQueueFamilies(std::vector<vk::DeviceQueueCreateInfo> &queuesInfo, std::vector<float> &queuesPriorities);
  void InitQueues();

  GraphicsVk *GetGraphics();
  vk::PhysicalDevice &GetPhysicalDevice();

  PhysicalDeviceVk *_physicalDevice;
  std::vector<vk::LayerProperties> _layers;
  std::vector<vk::ExtensionProperties> _extensions;

  vk::UniqueDevice _device;

  QueueVk _graphicsQueue;
  QueueVk _presentQueue;
  QueueVk _transferQueue;
  QueueVk _computeQueue;
  QueueVk _sparseOpQueue;

  std::recursive_mutex _mutex;

};

}