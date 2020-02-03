#pragma once

#include "../buffer.h"
#include "queue_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;

class BufferVk : public Buffer {
	RTTR_ENABLE(Buffer)
public:
  BufferVk(Device &device) : Buffer(device) {}

	void Init(Usage usage, std::shared_ptr<util::LayoutElement> const &layout) override;
	virtual std::shared_ptr<ResourceStateTransitionPass> CreateTransitionPass(ResourceState srcState, ResourceState dstState) override;

  void *Map() override;
  void Unmap() override;

public:
  static vk::BufferUsageFlags GetBufferUsage(Usage usage);

	struct StateInfo {
		vk::AccessFlags _access;
		vk::PipelineStageFlags _stages;
		QueueRole _queueRole = QueueRole::Invalid;

		bool IsValid() { return _stages && _queueRole != QueueRole::Invalid; }
	};

	inline StateInfo GetStateInfo(ResourceState state) { return GetStateInfo(state, _usage); }
	static StateInfo GetStateInfo(ResourceState state, Usage usage);

  vk::UniqueBuffer _buffer;
  UniqueVmaAllocation _memory;
};

NAMESPACE_END(gr1)

