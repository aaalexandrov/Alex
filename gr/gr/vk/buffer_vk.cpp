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

util::TypeInfo * BufferVk::GetType()
{
  return util::TypeInfo::Get<BufferVk>();
}

void * BufferVk::Map()
{
  return VmaMapMemory(*_device->_allocator, *_memory);
}

void BufferVk::Unmap()
{
  vmaUnmapMemory(*_device->_allocator, *_memory);
}

void BufferVk::RecordTransitionCommands(vk::CommandBuffer srcCommands, QueueVk *srcQueue, vk::CommandBuffer dstCommands, QueueVk *dstQueue, vk::PipelineStageFlags &dstStageFlags)
{
  std::array<vk::BufferMemoryBarrier, 1> srcBarrier;
  srcBarrier[0]
    .setSrcAccessMask(GetBufferAccess() & srcQueue->GetAccessFlags())
    .setSrcQueueFamilyIndex(srcQueue->_family)
    .setDstQueueFamilyIndex(dstQueue->_family)
    .setBuffer(_buffer.get())
    .setSize(GetSize());
  ASSERT(srcBarrier[0].srcAccessMask);
  auto srcStageFlags = GetBufferPipelineStage() & srcQueue->GetPipelineStageFlags();
  ASSERT(srcStageFlags);
  srcCommands.pipelineBarrier(srcStageFlags, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), nullptr, srcBarrier, nullptr);

  std::array<vk::BufferMemoryBarrier, 1> dstBarrier;
  dstBarrier[0]
    .setDstAccessMask(GetBufferAccess() & dstQueue->GetAccessFlags())
    .setSrcQueueFamilyIndex(srcQueue->_family)
    .setDstQueueFamilyIndex(dstQueue->_family)
    .setBuffer(_buffer.get())
    .setSize(GetSize());
  ASSERT(dstBarrier[0].dstAccessMask);
  dstStageFlags = GetBufferPipelineStage() & dstQueue->GetPipelineStageFlags();
  ASSERT(dstStageFlags);
  dstCommands.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, dstStageFlags, vk::DependencyFlags(), nullptr, dstBarrier, nullptr);
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

vk::AccessFlags BufferVk::GetBufferAccess(Usage usage)
{
  vk::AccessFlags flags;
  switch (usage) {
    case Usage::Vertex:
      flags = vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eTransferWrite;
      break;
    case Usage::Index:
      flags = vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eTransferWrite;
      break;
    case Usage::Uniform:
      flags = vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eTransferWrite;
      break;
    case Usage::Staging:
      flags = vk::AccessFlagBits::eHostRead | vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferRead;
      break;
    default:
      throw GraphicsException("Unsupported buffer usage!", VK_ERROR_INITIALIZATION_FAILED);
  }
  return flags;
}

vk::PipelineStageFlags BufferVk::GetBufferPipelineStage(Usage usage)
{
  vk::PipelineStageFlags flags;
  switch (usage) {
    case Usage::Vertex:
      flags = vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eTransfer;
      break;
    case Usage::Index:
      flags = vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eTransfer;
      break;
    case Usage::Uniform:
      flags = vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eTransfer;
      break;
    case Usage::Staging:
      flags = vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eHost;
      break;
    default:
      throw GraphicsException("Unsupported buffer usage!", VK_ERROR_INITIALIZATION_FAILED);
  }
  return vk::PipelineStageFlags();
}


NAMESPACE_END(gr)

