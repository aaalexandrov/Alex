#include "buffer_copy_pass_vk.h"
#include "device_vk.h"
#include "execution_queue_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<BufferCopyPassVk>("BufferCopyPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

BufferCopyPassVk::BufferCopyPassVk(Device &device)
	: BufferCopyPass(device)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_cmdCopy = deviceVk->TransferQueue().AllocateCmdBuffer();
}

void BufferCopyPassVk::Prepare()
{
	BufferVk *srcVk = static_cast<BufferVk*>(_src.get());
	BufferVk *dstVk = static_cast<BufferVk*>(_dst.get());

	std::lock_guard<CmdBufferVk> copyLock(_cmdCopy);

	_cmdCopy->reset(vk::CommandBufferResetFlags());

	_cmdCopy->begin(vk::CommandBufferBeginInfo());

	std::array<vk::BufferCopy, 1> copy;
	copy[0]
		.setSrcOffset(_srcOffset)
		.setDstOffset(_dstOffset)
		.setSize(_size);
	_cmdCopy->copyBuffer(*srcVk->_buffer, *dstVk->_buffer, copy);

	_cmdCopy->end();
}

void BufferCopyPassVk::Execute(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);
	std::vector<vk::Semaphore> signalSemaphores;
	FillDependencySemaphores(dependencies, DependencyType::Output, signalSemaphores);

	std::array<vk::SubmitInfo, 1> copySubmit;
	copySubmit[0]
		.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStageFlags.data())
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*_cmdCopy)
		.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
		.setPSignalSemaphores(signalSemaphores.data());
	deviceVk->TransferQueue()._queue.submit(copySubmit, nullptr);
}


NAMESPACE_END(gr1)

