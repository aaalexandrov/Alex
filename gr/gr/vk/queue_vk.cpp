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

vk::UniqueCommandBuffer &&QueueVk::AllocateCmdBuffer()
{
  vk::CommandBufferAllocateInfo cmdsInfo(*_cmdPool, vk::CommandBufferLevel::ePrimary, 1);
  std::vector<vk::UniqueCommandBuffer> buffers = _device->_device->allocateCommandBuffersUnique(cmdsInfo);
  return std::move(buffers[0]);
}

NAMESPACE_END(gr)