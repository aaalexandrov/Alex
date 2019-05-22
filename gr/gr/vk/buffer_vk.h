#pragma once

#include "util/namespace.h"
#include "../buffer.h"
#include "vk.h"
#include "vma.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

class BufferVk : public Buffer {
public:
  BufferVk(DeviceVk &device, size_t size, BufferDescPtr &bufferDesc, Usage usage);

  util::TypeInfo *GetType() override;

  Usage GetUsage() override { return _usage; }
  size_t GetSize() override { return _size; }

  void *Map() override;
  void Unmap() override;

  static vk::BufferUsageFlags Usage2Flags(Usage usage);

  Usage _usage;
  size_t _size;

  DeviceVk *_device;
  vk::UniqueBuffer _buffer;
  UniqueVmaAllocation _memory;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::BufferVk, gr::Buffer)