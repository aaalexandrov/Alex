#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

struct QueueVk {
  void Init(DeviceVk &device, int32_t family, int32_t queueIndex);

  vk::UniqueCommandBuffer &&AllocateCmdBuffer();

  DeviceVk* _device;
  int32_t _family;
  vk::Queue _queue;
  vk::UniqueCommandPool _cmdPool;
};

NAMESPACE_END(gr)