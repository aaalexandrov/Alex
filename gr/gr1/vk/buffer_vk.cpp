#include "buffer_vk.h"
#include "device_vk.h"
#include "../execution_queue.h"
#include "../graphics_exception.h"
#include "../rttr_factory.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<BufferVk>("BufferVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

void BufferVk::Init(Usage usage, BufferDescPtr &bufferDesc)
{
	_bufferDesc = bufferDesc;
	_usage = usage;
	_size = bufferDesc->_size;

	DeviceVk *deviceVk = GetDevice<DeviceVk>();
  vk::BufferCreateInfo bufInfo;
  bufInfo
    .setSize(_size)
    .setUsage(Usage2Flags(_usage))
    .setSharingMode(vk::SharingMode::eExclusive);
  _buffer = deviceVk->_device->createBufferUnique(bufInfo, deviceVk->AllocationCallbacks());

  vk::MemoryRequirements memReq = deviceVk->_device->getBufferMemoryRequirements(*_buffer);
  _memory = VmaAllocateMemoryUnique(*deviceVk->_allocator, memReq, usage == Usage::Staging);
  vmaBindBufferMemory(*deviceVk->_allocator, *_memory, *_buffer);
}

std::shared_ptr<ResourceStateTransitionPass> BufferVk::CreateTransitionPass(ResourceState srcState, ResourceState dstState)
{
	throw "Implament it!";
}

void *BufferVk::Map()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	return VmaMapMemory(*deviceVk->_allocator, *_memory);
}

void BufferVk::Unmap()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
  vmaUnmapMemory(*deviceVk->_allocator, *_memory);
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


NAMESPACE_END(gr1)

