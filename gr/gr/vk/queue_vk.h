#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

struct QueueVk {
  int32_t _family;
  vk::Queue _queue;

  vk::UniqueCommandPool _cmdPool;

  void Init(DeviceVk &device, int32_t family, int32_t queueIndex);
};

NAMESPACE_END(gr)