#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr1)

enum class QueueRole {
	First,
	Graphics = First,
	Compute,
	Transfer,
	SparseOp,
	Present,
	Last,
	Any = Last,
	Invalid,
};

struct CommandPoolVk;
using CmdBufferVk = OwnedUniqueHandle<CommandPoolVk, vk::UniqueCommandBuffer>;

class DeviceVk;
struct CommandPoolVk {
	void Init(DeviceVk &deviceVk, QueueRole role);

	CmdBufferVk AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

	std::recursive_mutex _mutex;
	vk::UniqueCommandPool _cmdPool;
};

struct QueueVk {
  void Init(DeviceVk &deviceVk, int32_t family, int32_t queueIndex, QueueRole role);

	CmdBufferVk AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  vk::PipelineStageFlags GetPipelineStageFlags() const { return GetPipelineStageFlags(_role); }
  static vk::PipelineStageFlags GetPipelineStageFlags(QueueRole role);
  vk::AccessFlags GetAccessFlags() const { return GetAccessFlags(_role); }
  static vk::AccessFlags GetAccessFlags(QueueRole role);

  DeviceVk *_deviceVk = nullptr;
  int32_t _family;
  QueueRole _role;
  vk::Queue _queue;
	std::array<CommandPoolVk, 11> _cmdPools;
};

NAMESPACE_END(gr1)