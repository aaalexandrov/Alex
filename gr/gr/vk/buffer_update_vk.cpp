#include "buffer_update_vk.h"
#include "buffer_vk.h"
#include "device_vk.h"
#include "graphics_vk.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

BufferUpdateVk::BufferUpdateVk(BufferVk &updatedBuffer, void *data, size_t size, ptrdiff_t offset)
  : _buffer(util::SharedFromThis<BufferVk>(&updatedBuffer))
  , _signalAfterUpdate{ updatedBuffer._device->CreateSemaphore() }
{
  GraphicsVk *graphics = updatedBuffer._device->GetGraphics();
  _stagingBuffer = std::static_pointer_cast<BufferVk>(graphics->CreateBuffer(Buffer::Usage::Staging, graphics->GetRawBufferDesc(), size));
  void *staging = _stagingBuffer->Map();
  memcpy(staging, data, size);
  _stagingBuffer->Unmap();

  RecordCommands(size, offset);
}

void BufferUpdateVk::RecordCommands(size_t size, ptrdiff_t offset)
{
  BufferVk &buffer = *_buffer;
  _transferCmds = buffer._device->_transferQueue.AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
  _transferCmds->begin(beginInfo);

  std::array<vk::BufferCopy, 1> bufferCopy { { { 0, static_cast<vk::DeviceSize>(offset), size } } };
  _transferCmds->copyBuffer(*_stagingBuffer->_buffer, *buffer._buffer, bufferCopy);

  _transferCmds->end();
}

NAMESPACE_END(gr)

