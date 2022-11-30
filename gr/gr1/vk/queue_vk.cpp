#include "queue_vk.h"
#include "../execution_queue.h"
#include "device_vk.h"
#include "../graphics_exception.h"
#include "util/mathutl.h"

NAMESPACE_BEGIN(gr1)

void QueueVk::Init(DeviceVk &deviceVk, int32_t family, int32_t queueIndex)
{
  _deviceVk = &deviceVk;
  _family = family;
  _queue = deviceVk._device->getQueue(family, queueIndex);

	for (auto &pool : _cmdPools) {
		pool.Init(deviceVk, _family);
	}
}

CmdBufferVk QueueVk::AllocateCmdBuffer(vk::CommandBufferLevel level)
{
	size_t threadHash = std::hash<std::thread::id>()(std::this_thread::get_id());
	size_t ind = threadHash % _cmdPools.size();
	//std::cout << "Allocating from thread " << std::this_thread::get_id() << " and pool " << ind << std::endl;
	return _cmdPools[ind].AllocateCmdBuffer(level);
}



void CommandPoolVk::Init(DeviceVk &deviceVk, int32_t queueFamily)
{
	vk::CommandPoolCreateInfo poolInfo{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer, static_cast<uint32_t>(queueFamily) };
	_cmdPool = deviceVk._device->createCommandPoolUnique(poolInfo, deviceVk.AllocationCallbacks());
}

CmdBufferVk CommandPoolVk::AllocateCmdBuffer(vk::CommandBufferLevel level)
{
	vk::CommandBufferAllocateInfo cmdsInfo(*_cmdPool, level, 1);
	std::lock_guard<std::recursive_mutex> allocLock(_mutex);
	std::vector<vk::UniqueCommandBuffer> buffers = _cmdPool.getOwner().allocateCommandBuffersUnique(cmdsInfo);
	return CmdBufferVk(this, std::move(buffers[0]));
}


NAMESPACE_END(gr1)

