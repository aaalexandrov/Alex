#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr1)

enum class QueueRole {
	Graphics,
	Compute,
	Transfer,
	SparseOp,
	Present,
	Any,
	First = Graphics,
	Last = Present,
	Count = Last - First + 1,
	Invalid,
};

struct CommandPoolVk;
using CmdBufferVk = OwnedUniqueHandle<CommandPoolVk, vk::UniqueCommandBuffer>;

class DeviceVk;
struct CommandPoolVk {
	void Init(DeviceVk &deviceVk, int32_t queueFamily);

	CmdBufferVk AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

	std::recursive_mutex _mutex;
	vk::UniqueCommandPool _cmdPool;
};

struct QueueVk {
  void Init(DeviceVk &deviceVk, int32_t family, int32_t queueIndex);

  CmdBufferVk AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  DeviceVk *_deviceVk = nullptr;
  int32_t _family;
  vk::Queue _queue;
  std::array<CommandPoolVk, 11> _cmdPools;
};

NAMESPACE_END(gr1)