#pragma once

#include "vk.h"
#include "vma.h"
#include "queue_vk.h"
#include <mutex>


NAMESPACE_BEGIN(gr)

class GraphicsVk;
class PhysicalDeviceVk;
class ModelInstance;

class DeviceVk {
public:
  DeviceVk(PhysicalDeviceVk *physicalDevice);

  void InitQueueFamilies(std::vector<vk::DeviceQueueCreateInfo> &queuesInfo, std::vector<float> &queuesPriorities);
  void InitQueues();

  vk::UniqueSemaphore CreateSemaphore();

  void RenderInstance(std::shared_ptr<ModelInstance> &modelInst);

  GraphicsVk *GetGraphics();
  vk::AllocationCallbacks *AllocationCallbacks();
  vk::PhysicalDevice &GetPhysicalDevice();

  PhysicalDeviceVk *_physicalDevice;
  std::vector<vk::LayerProperties> _layers;
  std::vector<vk::ExtensionProperties> _extensions;

  vk::UniqueDevice _device;
  UniqueVmaAllocator _allocator;

  QueueVk _graphicsQueue;
  QueueVk _presentQueue;
  QueueVk _transferQueue;
  QueueVk _computeQueue;
  QueueVk _sparseOpQueue;

  std::recursive_mutex _mutex;

};

NAMESPACE_END(gr)