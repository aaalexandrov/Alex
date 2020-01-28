#include "execution_queue_vk.h"
#include "../graphics_exception.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

ExecutionQueueVk::ExecutionQueueVk(DeviceVk &deviceVk)
	: ExecutionQueue(deviceVk)
{

}

void ExecutionQueueVk::ExecutePasses()
{
	PreparePassData();

	Prepare();

	Execute();

	WaitExecutionFinished();

	_passes.clear();
	_passData.clear();
}

void ExecutionQueueVk::WaitExecutionFinished()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	PassVk *lastPassVk = rttr::rttr_cast<PassVk*>(_passes.back().get());
	vk::Result result = deviceVk->_device->waitForFences(1, &*lastPassVk->_signalFence, true, std::numeric_limits<uint64_t>::max());
	if (result != vk::Result::eSuccess)
		throw GraphicsException("WaitExecutionFinished() failed!", (uint32_t)result);
}

void ExecutionQueueVk::PreparePassData()
{
	ASSERT(!_passData.size());
	for (uint32_t i = 0; i < _passes.size(); ++i) {
		_passData.emplace_back();
		if (i > 0) {
			PassVk *prevPassVk = rttr::rttr_cast<PassVk*>(_passes[i - 1].get());
			_passData.back()._waitSemaphore = *prevPassVk->_signalSemaphore;
			_passData.back()._waitFence = *prevPassVk->_signalFence;
		}
	}
}

void ExecutionQueueVk::Prepare()
{
	for (uint32_t i = 0; i < _passes.size(); ++i) {
		_passes[i]->Prepare(&_passData[i]);
	}
}

void ExecutionQueueVk::Execute()
{
	for (uint32_t i = 0; i < _passes.size(); ++i) {
		_passes[i]->Execute(&_passData[i]);
	}
}


NAMESPACE_END(gr1)

