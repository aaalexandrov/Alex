#include "execution_queue_vk.h"
#include "../graphics_exception.h"

NAMESPACE_BEGIN(gr1)

PassDependencyVk::PassDependencyVk(DeviceVk &deviceVk, OutputPass *srcPass, OutputPass *dstPass) 
	: PassDependency(srcPass, dstPass) 
{
	_semaphore = deviceVk.CreateSemaphore();
}

ExecutionQueueVk::ExecutionQueueVk(DeviceVk &deviceVk)
	: ExecutionQueue(deviceVk)
{
	_dependencyTracker.SetPassDependencyCreator([&deviceVk](OutputPass *srcPass, OutputPass *dstPass) { 
		return std::make_unique<PassDependencyVk>(deviceVk, srcPass, dstPass); 
	});
}

NAMESPACE_END(gr1)

