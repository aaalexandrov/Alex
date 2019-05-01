#pragma once

#include "vk.h"

namespace gr {

class DeviceVk;

struct QueueVk {
  int32_t _family;
  vk::Queue _queue;

  vk::UniqueCommandPool _cmdPool;

  void Init(DeviceVk &device, int32_t family, int32_t queueIndex);
};

}