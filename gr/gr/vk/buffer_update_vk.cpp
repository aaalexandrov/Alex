#include "buffer_update_vk.h"
#include "buffer_vk.h"
#include "device_vk.h"
#include "graphics_vk.h"

NAMESPACE_BEGIN(gr)

BufferUpdateVk::BufferUpdateVk(BufferVk &updatedBuffer, void *data, size_t size, ptrdiff_t offset)
  : ResourceUpdate(updatedBuffer)
  , _signalAfterUpdate{ updatedBuffer._device->CreateSemaphore() }
{
  if (!updatedBuffer._lastUpdate.expired()) {
    auto lastUpdate = updatedBuffer._lastUpdate.lock();
    BufferUpdateVk *prevUpdate = static_cast<BufferUpdateVk *>(lastUpdate.get());
    _waitBeforeUpdate = prevUpdate->_signalAfterUpdate;
  }

  GraphicsVk *graphics = updatedBuffer._device->GetGraphics();
  _stagingBuffer = std::static_pointer_cast<BufferVk>(graphics->CreateBuffer(Buffer::Usage::Staging, graphics->GetRawBufferDesc(), size));
  void *staging = _stagingBuffer->Map();
  memcpy(staging, data, size);
  _stagingBuffer->Unmap();

  RecordCommands(size, offset);

  updatedBuffer._lastUpdate = std::static_pointer_cast<ResourceUpdate>(shared_from_this());
}

BufferVk &BufferUpdateVk::GetBuffer()
{
  return *static_cast<BufferVk*>(_resource.get());
}

void BufferUpdateVk::RecordCommands(size_t size, ptrdiff_t offset)
{
  BufferVk &buffer = GetBuffer();
  _transferCmds = buffer._device->_transferQueue.AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
  _transferCmds->begin(beginInfo);

  std::array<vk::BufferCopy, 1> bufferCopy { { { 0, static_cast<vk::DeviceSize>(offset), size } } };
  _transferCmds->copyBuffer(*_stagingBuffer->_buffer, *buffer._buffer, bufferCopy);

  _transferCmds->end();
}

NAMESPACE_END(gr)

