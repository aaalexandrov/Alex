#include "buffer_update_vk.h"
#include "buffer_vk.h"
#include "device_vk.h"
#include "graphics_vk.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

BufferUpdateVk::BufferUpdateVk(BufferVk &updatedBuffer, void *data, size_t size, ptrdiff_t offset)
  : _buffer{ util::SharedFromThis<BufferVk>(&updatedBuffer) }
  , _offset{ offset }
{
  CreateStagingBuffer(data, size);
}

void BufferUpdateVk::CreateStagingBuffer(void *data, size_t size)
{
  GraphicsVk *graphics = _buffer->_device->GetGraphics();
  _stagingBuffer = graphics->CreateBufferTyped<BufferVk>(Buffer::Usage::Staging, graphics->GetRawBufferDesc(), size);
  void *staging = _stagingBuffer->Map();
  memcpy(staging, data, size);
  _stagingBuffer->Unmap();
}

void BufferUpdateVk::Prepare()
{
  DeviceVk *device = _buffer->_device;
  _queue = &device->_transferQueue;
  _semaphoreDone = device->CreateSemaphore();

  _commands = _queue->AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
  _commands->begin(beginInfo);

  std::array<vk::BufferCopy, 1> bufferCopy { { { 0, static_cast<vk::DeviceSize>(_offset), _stagingBuffer->GetSize() } } };
  _commands->copyBuffer(*_stagingBuffer->_buffer, *_buffer->_buffer, bufferCopy);

  _commands->end();
}

NAMESPACE_END(gr)

