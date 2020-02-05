#pragma once

#include "device_vk.h"
#include "../execution_queue.h"
#include "../output_pass.h"

NAMESPACE_BEGIN(gr1)

class PassDependencyVk : public PassDependency {
	RTTR_ENABLE(PassDependency)
public:
	PassDependencyVk(DeviceVk &deviceVk, OutputPass *srcPass, OutputPass *dstPass);

	vk::UniqueSemaphore _semaphore;
};

class ExecutionQueueVk : public ExecutionQueue {
	RTTR_ENABLE(ExecutionQueue)
public:
	ExecutionQueueVk(DeviceVk &deviceVk);

protected:

};

NAMESPACE_END(gr1)