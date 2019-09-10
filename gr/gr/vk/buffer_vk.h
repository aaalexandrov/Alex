#pragma once

#include "util/namespace.h"
#include "../buffer.h"
#include "owned_by_queue_vk.h"
#include "vk.h"
#include "vma.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

class BufferVk : public Buffer, public OwnedByQueueVk<BufferVk> {
public:
  BufferVk(DeviceVk &device, size_t size, BufferDescPtr &bufferDesc, Usage usage);

  util::TypeInfo *GetType() override;

  Usage GetUsage() override { return _usage; }
  size_t GetSize() override { return _size; }

  void *Map() override;
  void Unmap() override;

  void RecordTransitionCommands(vk::CommandBuffer srcCommands, QueueVk *srcQueue, vk::CommandBuffer dstCommands, QueueVk *dstQueue, vk::PipelineStageFlags &dstStageFlags);

  static vk::BufferUsageFlags Usage2Flags(Usage usage);
  vk::AccessFlags GetBufferAccess() const { return GetBufferAccess(_usage); }
  static vk::AccessFlags GetBufferAccess(Usage usage);
  vk::PipelineStageFlags GetBufferPipelineStage() const { return GetBufferPipelineStage(_usage); }
  static vk::PipelineStageFlags GetBufferPipelineStage(Usage usage);

  Usage _usage;
  size_t _size;

  DeviceVk *_device;
  vk::UniqueBuffer _buffer;
  UniqueVmaAllocation _memory;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::OwnedByQueueVk<gr::BufferVk>)
RTTI_BIND(gr::BufferVk, gr::Buffer, gr::OwnedByQueueVk<gr::BufferVk>)