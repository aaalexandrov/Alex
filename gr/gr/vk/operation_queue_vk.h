#pragma once

#include "../operation_queue.h"
#include "graphics_vk.h"
#include "device_vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class PresentationSurfaceVk;

class OperationQueueVk : public OperationQueue {
public:
  OperationQueueVk(DeviceVk &device) : _device{&device} {}
  
  GraphicsVk *GetGraphics() const override;

  PresentationSurfaceVk *GetPresentationSurfaceVk();

  void ClearOperations() override;

  void PreProcessOperations() override;
  void PostProcessOperations() override;

  DeviceVk *_device;
  std::map<QueueVk*, vk::UniqueFence> _queueFences;
};

NAMESPACE_END(gr)