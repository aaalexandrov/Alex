#pragma once

#include "../graphics_resource.h"
#include "vk.h"

NAMESPACE_BEGIN(gr)

class BufferVk;

class BufferUpdateVk : public ResourceUpdate {
public:
  BufferUpdateVk(BufferVk &updatedBuffer, void *data, size_t size, ptrdiff_t offset);

  BufferVk &GetBuffer();

  void RecordCommands(size_t size, ptrdiff_t offset);

  std::shared_ptr<BufferVk> _stagingBuffer;
  std::shared_ptr<vk::UniqueSemaphore> _waitBeforeUpdate;
  vk::UniqueCommandBuffer _transferCmds;
  std::shared_ptr<vk::UniqueSemaphore> _signalAfterUpdate;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::BufferUpdateVk, gr::ResourceUpdate)