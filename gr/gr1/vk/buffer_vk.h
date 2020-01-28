#pragma once

#include "../buffer.h"
#include "rttr/registration.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;

class BufferVk : public Buffer {
	RTTR_ENABLE(Buffer)
public:
  BufferVk(Device &device) : Buffer(device) {}

	void Init(Usage usage, BufferDescPtr &bufferDesc) override;

	rttr::type GetStateTransitionPassType() override { throw "Unimplemented"; }

  inline Usage GetUsage() override { return _usage; }
  inline size_t GetSize() override { return _size; }

  void *Map() override;
  void Unmap() override;

protected:
  static vk::BufferUsageFlags Usage2Flags(Usage usage);
  vk::AccessFlags GetBufferAccess() const { return GetBufferAccess(_usage); }
  static vk::AccessFlags GetBufferAccess(Usage usage);
  vk::PipelineStageFlags GetBufferPipelineStage() const { return GetBufferPipelineStage(_usage); }
  static vk::PipelineStageFlags GetBufferPipelineStage(Usage usage);

  Usage _usage;
  size_t _size;

  vk::UniqueBuffer _buffer;
  UniqueVmaAllocation _memory;
};

NAMESPACE_END(gr1)

