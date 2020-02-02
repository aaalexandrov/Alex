#include "execution_queue_vk.h"
#include "../graphics_exception.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

ExecutionQueueVk::ExecutionQueueVk(DeviceVk &deviceVk)
	: ExecutionQueue(deviceVk)
{
	_passesFinished = deviceVk.CreateFence();

	_cmdFinish = deviceVk.GraphicsQueue().AllocateCmdBuffer();
	_cmdFinish->begin(vk::CommandBufferBeginInfo());
	_cmdFinish->end();
}

void ExecutionQueueVk::WaitExecutionFinished()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	PassVk *lastPassVk = rttr::rttr_cast<PassVk*>(_passes.back()->_pass.get());

	std::array<vk::Fence, 1> passFence{ *_passesFinished };
	deviceVk->_device->resetFences(passFence);

	std::array<vk::SubmitInfo, 1> submit;
	submit[0]
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&*lastPassVk->_signalSemaphore)
		.setPWaitDstStageMask(&lastPassVk->_signalSemaphoreStages)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*_cmdFinish);
	deviceVk->GraphicsQueue()._queue.submit(submit, *_passesFinished);

	vk::Result result = deviceVk->_device->waitForFences(passFence, true, std::numeric_limits<uint64_t>::max());
	if (result != vk::Result::eSuccess)
		throw GraphicsException("WaitExecutionFinished() failed!", (uint32_t)result);
}

void ExecutionQueueVk::Prepare()
{
	for (uint32_t i = 0; i < _passes.size(); ++i) {
		for (auto &transition : _passes[i]->_transitionPasses) {
			transition->_pass->Prepare(transition.get());
		}

		_passes[i]->_pass->Prepare(_passes[i].get());
	}
}

void ExecutionQueueVk::Execute()
{
	for (uint32_t i = 0; i < _passes.size(); ++i) {
		for (auto &transition : _passes[i]->_transitionPasses) {
			transition->_pass->Execute(transition.get());
		}

		_passes[i]->_pass->Execute(_passes[i].get());
	}
}


NAMESPACE_END(gr1)

