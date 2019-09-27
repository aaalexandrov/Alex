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
  vk::UniqueFence CreateFence(vk::FenceCreateFlags flags = vk::FenceCreateFlags());

  void RenderInstance(std::shared_ptr<ModelInstance> &modelInst);

  GraphicsVk *GetGraphics();
  vk::AllocationCallbacks *AllocationCallbacks();
  vk::PhysicalDevice &GetPhysicalDevice();

  QueueVk &Queue(QueueRole role) { return _queues[static_cast<int>(role)]; }

  QueueVk &GraphicsQueue() { return Queue(QueueRole::Graphics); }
  QueueVk &PresentQueue() { return Queue(QueueRole::Present); }
  QueueVk &TransferQueue() { return Queue(QueueRole::Transfer); }
  QueueVk &ComputeQueue() { return Queue(QueueRole::Compute); }
  QueueVk &SparseOpQueue() { return Queue(QueueRole::SparseOp); }

  PhysicalDeviceVk *_physicalDevice;
  std::vector<vk::LayerProperties> _layers;
  std::vector<vk::ExtensionProperties> _extensions;

  vk::UniqueDevice _device;
  UniqueVmaAllocator _allocator;

  std::array<QueueVk, static_cast<int>(QueueRole::Count)> _queues;

  std::recursive_mutex _mutex;
};

NAMESPACE_END(gr)