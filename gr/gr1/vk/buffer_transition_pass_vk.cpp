#include "buffer_transition_pass_vk.h"
#include "device_vk.h"
#include "buffer_vk.h"
#include "execution_queue_vk.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

BufferTransitionPassVk::BufferTransitionPassVk(Device &device)
	: ResourceStateTransitionPass(device)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	_queueTransitionSemaphore = deviceVk->CreateSemaphore();
}

void BufferTransitionPassVk::Prepare()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	BufferVk *buf = GetResource<BufferVk>();

	BufferVk::StateInfo srcState = buf->GetStateInfo(_srcState);
	BufferVk::StateInfo dstState = buf->GetStateInfo(_dstState);

	QueueVk *srcQueue, *dstQueue;
	bool queueTransition = GetTransitionQueueInfo(deviceVk, srcState._queueRole, dstState._queueRole, srcQueue, dstQueue);

	std::array<vk::BufferMemoryBarrier, 1> bufBarrier;

	_srcCmds = srcQueue->AllocateCmdBuffer();

	{
		std::lock_guard<CmdBufferVk> srcLock(_srcCmds);

		_srcCmds->begin(vk::CommandBufferBeginInfo());

		bufBarrier[0]
			.setSrcAccessMask(srcState._access)
			.setDstAccessMask(queueTransition ? vk::AccessFlags() : dstState._access)
			.setSrcQueueFamilyIndex(queueTransition ? srcQueue->_family : VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(queueTransition ? dstQueue->_family : VK_QUEUE_FAMILY_IGNORED)
			.setBuffer(*buf->_buffer)
			.setOffset(0)
			.setSize(buf->GetSize());
		_srcCmds->pipelineBarrier(srcState._stages, queueTransition ? vk::PipelineStageFlagBits::eBottomOfPipe : dstState._stages,
			vk::DependencyFlags(), nullptr, bufBarrier, nullptr);

		_srcCmds->end();
	}

	if (queueTransition) {

		_dstCmds = dstQueue->AllocateCmdBuffer();

		std::lock_guard<CmdBufferVk> dstLock(_dstCmds);

		_dstCmds->begin(vk::CommandBufferBeginInfo());

		bufBarrier[0]
			.setSrcAccessMask(vk::AccessFlags())
			.setDstAccessMask(dstState._access)
			.setSrcQueueFamilyIndex(srcQueue->_family)
			.setDstQueueFamilyIndex(dstQueue->_family)
			.setBuffer(*buf->_buffer)
			.setOffset(0)
			.setSize(buf->GetSize());
		_dstCmds->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, dstState._stages,
			vk::DependencyFlags(), nullptr, bufBarrier, nullptr);

		_dstCmds->end();
	}
}

void BufferTransitionPassVk::Submit(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	BufferVk *buf = GetResource<BufferVk>();

	BufferVk::StateInfo srcState = buf->GetStateInfo(_srcState);
	BufferVk::StateInfo dstState = buf->GetStateInfo(_dstState);

	QueueVk *srcQueue, *dstQueue;
	bool queueTransition = GetTransitionQueueInfo(deviceVk, srcState._queueRole, dstState._queueRole, srcQueue, dstQueue);

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);
	std::vector<vk::Semaphore> signalSemaphores;
	FillDependencySemaphores(dependencies, DependencyType::Output, signalSemaphores);

	std::array<vk::SubmitInfo, 1> srcSubmit, dstSubmit;

	if (queueTransition) {

		srcSubmit[0]
			.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_srcCmds)
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&*_queueTransitionSemaphore);
		srcQueue->_queue.submit(srcSubmit, nullptr);


		waitSemaphores.clear();
		waitStageFlags.clear();
		waitSemaphores.push_back(*_queueTransitionSemaphore);
		waitStageFlags.push_back(dstState._stages);

		dstSubmit[0]
			.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_dstCmds)
			.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
			.setPSignalSemaphores(signalSemaphores.data());
		dstQueue->_queue.submit(dstSubmit, nullptr);

	} else {

		srcSubmit[0]
			.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_srcCmds)
			.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
			.setPSignalSemaphores(signalSemaphores.data());
		srcQueue->_queue.submit(srcSubmit, nullptr);

	}
}

vk::PipelineStageFlags BufferTransitionPassVk::GetPassDstStages()
{
	BufferVk *buf = GetResource<BufferVk>();
	BufferVk::StateInfo dstState = buf->GetStateInfo(_dstState);
	return dstState._stages & ~vk::PipelineStageFlagBits::eHost;
}

NAMESPACE_END(gr1)

