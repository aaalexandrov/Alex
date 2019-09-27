#pragma once

#include "../operation_queue.h"
#include "graphics_vk.h"
#include "device_vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class PresentationSurfaceVk;

class OperationQueueVk : public OperationQueue {
public:
  OperationQueueVk(DeviceVk &device);
  
  GraphicsVk *GetGraphics() const override;

  PresentationSurfaceVk *GetPresentationSurfaceVk();

  void ClearOperations() override;

  void PreProcessOperations() override;
  void PostProcessOperations() override;

  void WaitOperationsEnd() override;

  vk::Fence QueueFence(QueueRole role) { return _queueFences[static_cast<int>(role)].get(); }

  DeviceVk *_device;
  std::array<vk::UniqueFence, static_cast<int>(QueueRole::Count)> _queueFences;
};

NAMESPACE_END(gr)