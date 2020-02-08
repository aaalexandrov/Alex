#include "buffer_vk.h"
#include "device_vk.h"
#include "buffer_transition_pass_vk.h"
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

void BufferVk::Init(Usage usage, std::shared_ptr<util::LayoutElement> const &layout)
{
	Buffer::Init(usage, layout);

	DeviceVk *deviceVk = GetDevice<DeviceVk>();
  vk::BufferCreateInfo bufInfo;
  bufInfo
    .setSize(GetSize())
    .setUsage(GetBufferUsage(_usage))
    .setSharingMode(vk::SharingMode::eExclusive);
  _buffer = deviceVk->_device->createBufferUnique(bufInfo, deviceVk->AllocationCallbacks());

  vk::MemoryRequirements memReq = deviceVk->_device->getBufferMemoryRequirements(*_buffer);
  _memory = VmaAllocateMemoryUnique(*deviceVk->_allocator, memReq, usage == Usage::Staging);
  vmaBindBufferMemory(*deviceVk->_allocator, *_memory, *_buffer);
}

std::shared_ptr<ResourceStateTransitionPass> BufferVk::CreateTransitionPass(ResourceState srcState, ResourceState dstState)
{
	auto transition = std::make_shared<BufferTransitionPassVk>(*_device);
	transition->Init(AsSharedPtr<Resource>(), srcState, dstState);
	return std::move(transition);
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

vk::IndexType BufferVk::GetVkIndexType()
{
	ASSERT(_usage & Usage::Index);
	rttr::type elemType = _layout->GetArrayElement()->GetValueType();
	if (elemType == rttr::type::get<uint16_t>())
		return vk::IndexType::eUint16;
	if (elemType == rttr::type::get<uint32_t>())
		return vk::IndexType::eUint32;
	throw GraphicsException("Invalid index buffer content type!", VK_ERROR_FORMAT_NOT_SUPPORTED);
}

vk::BufferUsageFlags BufferVk::GetBufferUsage(Usage usage)
{
  vk::BufferUsageFlags flags;
	if (!!(usage & Usage::Vertex))
		flags |= vk::BufferUsageFlagBits::eVertexBuffer;
	if (!!(usage & Usage::Index))
		flags |= vk::BufferUsageFlagBits::eIndexBuffer;
	if (!!(usage & Usage::Uniform))
		flags |= vk::BufferUsageFlagBits::eUniformBuffer;
	if (!!(usage & Usage::Staging))
		flags |= vk::BufferUsageFlagBits::eTransferSrc;
	if (!flags)
		throw GraphicsException("BufferVk::GetImageUsage(): Unsupported usage value " + static_cast<uint32_t>(usage), VK_RESULT_MAX_ENUM);
	flags |= vk::BufferUsageFlagBits::eTransferDst;
	return flags;
}

BufferVk::StateInfo BufferVk::GetStateInfo(ResourceState state, Usage usage)
{
	StateInfo info;
	switch (state) {
		case ResourceState::Initial:
			if (!!(usage & Usage::Staging)) {
				info._access = vk::AccessFlagBits::eHostRead | vk::AccessFlagBits::eHostWrite;
				info._stages = vk::PipelineStageFlagBits::eHost;
				info._queueRole = QueueRole::Any;
			} else {
				info._access = vk::AccessFlags();
				info._stages = vk::PipelineStageFlagBits::eTopOfPipe;
				info._queueRole = QueueRole::Any;
			}
			break;
		case ResourceState::ShaderRead:
			if (!(usage & Usage::Staging)) {
				if (!!(usage & Usage::Vertex)) {
					info._access |= vk::AccessFlagBits::eVertexAttributeRead;
					info._stages |= vk::PipelineStageFlagBits::eVertexInput;
				}
				if (!!(usage & Usage::Index)) {
					info._access |= vk::AccessFlagBits::eIndexRead;
					info._stages |= vk::PipelineStageFlagBits::eVertexInput;
				}
				if (!!(usage & Usage::Uniform)) {
					info._access |= vk::AccessFlagBits::eUniformRead;
					info._stages |= vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
				}
				info._queueRole = QueueRole::Graphics;
			}
			break;
		case ResourceState::TransferRead:
			info._access = vk::AccessFlagBits::eTransferRead;
			info._stages = vk::PipelineStageFlagBits::eTransfer;
			info._queueRole = QueueRole::Transfer;
			if (!!(usage & Usage::Staging)) {
				info._access |= vk::AccessFlagBits::eHostRead | vk::AccessFlagBits::eHostWrite;
				info._stages |= vk::PipelineStageFlagBits::eHost;
			}
			break;
		case ResourceState::TransferWrite:
			info._access = vk::AccessFlagBits::eTransferWrite;
			info._stages = vk::PipelineStageFlagBits::eTransfer;
			info._queueRole = QueueRole::Transfer;
			if (!!(usage & Usage::Staging)) {
				info._access |= vk::AccessFlagBits::eHostRead | vk::AccessFlagBits::eHostWrite;
				info._stages |= vk::PipelineStageFlagBits::eHost;
			}
			break;
	}
	if (!info.IsValid())
		throw GraphicsException("BufferVk::GetStateInfo(): Unsupported state and usage combination", VK_RESULT_MAX_ENUM);
	return info;
}


NAMESPACE_END(gr1)

