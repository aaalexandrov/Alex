#include "buffer_update_vk.h"
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

void BufferUpdateVk::Prepare(OperationQueue *operationQueue)
{
  DeviceVk *device = _buffer->_device;

  _transitionToTransfer = std::move(_buffer->GetQueueTransitionTo(&device->_transferQueue));

  _transferCmds = device->_transferQueue.AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  _transferCmds->begin(beginInfo);

  std::array<vk::BufferCopy, 1> bufferCopy { { { 0, static_cast<vk::DeviceSize>(_offset), _stagingBuffer->GetSize() } } };
  _transferCmds->copyBuffer(*_stagingBuffer->_buffer, *_buffer->_buffer, bufferCopy);
}

void BufferUpdateVk::Execute(OperationQueue *operationQueue)
{
  if (_transitionToTransfer)
    _transitionToTransfer->ExecuteQueueTransition(operationQueue);
  
  DeviceVk *device = _buffer->_device;

  std::array<vk::SubmitInfo, 1> transferSubmit;
  transferSubmit[0]
    .setCommandBufferCount(1)
    .setPCommandBuffers(&_transferCmds.get());

  device->_transferQueue._queue.submit(transferSubmit, nullptr);
}

NAMESPACE_END(gr)

