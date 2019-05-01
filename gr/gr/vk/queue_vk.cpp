#include "queue_vk.h"
#include "device_vk.h"

namespace gr {

void QueueVk::Init(DeviceVk &device, int32_t family, int32_t queueIndex)
{
  _family = family;
  _queue = device._device->getQueue(family, queueIndex);

  vk::CommandPoolCreateInfo poolInfo { vk::CommandPoolCreateFlags(), static_cast<uint32_t>(family) };
  _cmdPool = device._device->createCommandPoolUnique(poolInfo);
}

}