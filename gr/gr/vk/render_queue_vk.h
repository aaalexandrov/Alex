#pragma once

#include "../render_queue.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class PresentationSurfaceVk;

class RenderQueueVk : public RenderQueue {
public:
  RenderQueueVk(DeviceVk &device) : _device{&device} {}
  
  void Render() override;

  PresentationSurfaceVk *GetPresentationSurfaceVk();

  DeviceVk *_device;
};

NAMESPACE_END(gr)