#pragma once

#include "../operation_queue.h"
#include "graphics_vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class PresentationSurfaceVk;

class OperationQueueVk : public OperationQueue {
public:
  OperationQueueVk(DeviceVk &device) : _device{&device} {}
  
  GraphicsVk *GetGraphics() const override;

  PresentationSurfaceVk *GetPresentationSurfaceVk();

  void ExecuteOperation(QueueOperation *operation, std::vector<QueueOperation*> const &dependsOn) override;

  DeviceVk *_device;
};

NAMESPACE_END(gr)