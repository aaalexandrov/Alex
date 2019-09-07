#pragma once

#include "queue_operation_vk.h"
#include "vk.h"

NAMESPACE_BEGIN(gr)

class BufferVk;

class BufferUpdateVk : public QueueOperationVk {
public:
  BufferUpdateVk(BufferVk &updatedBuffer, void *data, size_t size, ptrdiff_t offset);

  void RecordCommands(size_t size, ptrdiff_t offset);

  std::shared_ptr<BufferVk> _buffer;
  std::shared_ptr<BufferVk> _stagingBuffer;
  vk::UniqueCommandBuffer _transferCmds;
  std::shared_ptr<vk::UniqueSemaphore> _signalAfterUpdate;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::BufferUpdateVk, gr::QueueOperationVk)