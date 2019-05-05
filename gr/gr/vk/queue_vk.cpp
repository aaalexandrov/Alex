#include "queue_vk.h"
#include "device_vk.h"

NAMESPACE_BEGIN(gr)

void QueueVk::Init(DeviceVk &device, int32_t family, int32_t queueIndex)
{
  _family = family;
  _queue = device._device->getQueue(family, queueIndex);

  vk::CommandPoolCreateInfo poolInfo { vk::CommandPoolCreateFlagBits::eResetCommandBuffer, static_cast<uint32_t>(family) };
  _cmdPool = device._device->createCommandPoolUnique(poolInfo);
}

NAMESPACE_END(gr)