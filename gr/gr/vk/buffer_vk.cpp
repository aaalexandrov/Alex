#include "buffer_vk.h"
#include "../graphics_exception.h"
#include "device_vk.h"
#include "graphics_vk.h"

NAMESPACE_BEGIN(gr)

BufferVk::BufferVk(DeviceVk &device, size_t size, BufferDescPtr &bufferDesc, Usage usage)
  : Buffer(bufferDesc)
  , _device { &device }
  , _usage { usage }
  , _size { size }
{
  vk::BufferCreateInfo bufInfo;
  bufInfo
    .setSize(_size)
    .setUsage(Usage2Flags(_usage))
    .setSharingMode(vk::SharingMode::eExclusive);
  _buffer = _device->_device->createBufferUnique(bufInfo, _device->AllocationCallbacks());

  vk::MemoryRequirements memReq = _device->_device->getBufferMemoryRequirements(*_buffer);
  _memory = VmaAllocateMemoryUnique(*_device->_allocator, memReq, usage == Usage::Staging);
  vmaBindBufferMemory(*_device->_allocator, *_memory, *_buffer);
}

void * BufferVk::Map()
{
  return VmaMapMemory(*_device->_allocator, *_memory);
}

void BufferVk::Unmap()
{
  vmaUnmapMemory(*_device->_allocator, *_memory);
}

vk::BufferUsageFlags BufferVk::Usage2Flags(Usage usage)
{
  vk::BufferUsageFlags flags;
  switch (usage) {
    case Usage::Vertex:
      flags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
      break;
    case Usage::Index:
      flags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
      break;
    case Usage::Uniform:
      flags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst;
      break;
    case Usage::Staging:
      flags = vk::BufferUsageFlagBits::eTransferSrc;
      break;
    default:
      throw GraphicsException("Unsupported buffer usage!", VK_ERROR_INITIALIZATION_FAILED);
  }
  return flags;
}


NAMESPACE_END(gr)

