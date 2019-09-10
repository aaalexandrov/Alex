#pragma once

#include "queue_operation_vk.h"
#include "buffer_vk.h"

NAMESPACE_BEGIN(gr)

class BufferUpdateVk : public QueueOperationVk {
public:
  BufferUpdateVk(BufferVk &updatedBuffer, void *data, size_t size, ptrdiff_t offset);

  void Prepare(OperationQueue *operationQueue) override;
  void Execute(OperationQueue *operationQueue) override;

  void CreateStagingBuffer(void *data, size_t size);

  std::shared_ptr<BufferVk> _buffer;
  std::shared_ptr<BufferVk> _stagingBuffer;
  vk::UniqueCommandBuffer _transferCmds;
  std::unique_ptr<BufferVk::QueueTransition> _transitionToTransfer;
  ptrdiff_t _offset;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::BufferUpdateVk, gr::QueueOperationVk)