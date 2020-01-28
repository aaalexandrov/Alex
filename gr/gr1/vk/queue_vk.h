#pragma once

#include "vk.h"
#include "util/rect.h"

NAMESPACE_BEGIN(gr1)

enum class QueueRole {
	First,
	Graphics = First,
	Compute,
	Transfer,
	SparseOp,
	Present,
	Invalid,
	Count = Invalid,
};

class DeviceVk;

struct QueueVk {
  void Init(DeviceVk &device, int32_t family, int32_t queueIndex, QueueRole role);

  vk::UniqueCommandBuffer AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  vk::PipelineStageFlags GetPipelineStageFlags() const { return GetPipelineStageFlags(_role); }
  static vk::PipelineStageFlags GetPipelineStageFlags(QueueRole role);
  vk::AccessFlags GetAccessFlags() const { return GetAccessFlags(_role); }
  static vk::AccessFlags GetAccessFlags(QueueRole role);

  DeviceVk *_device = nullptr;
  int32_t _family;
  QueueRole _role;
  vk::Queue _queue;
  vk::UniqueCommandPool _cmdPool;
};

struct CmdBufferVk {
	CmdBufferVk(QueueVk &queue, vk::CommandBufferLevel level);

	QueueVk *_queue;
	vk::UniqueCommandBuffer _cmdBuffer;
	vk::UniqueSemaphore _semaphore;
	vk::UniqueFence _fence;
};

NAMESPACE_END(gr1)