#pragma once

#include "device_vk.h"
#include "../execution_queue.h"
#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class ExecutionQueueVk : public ExecutionQueue {
	RTTR_ENABLE(ExecutionQueue)
public:
	ExecutionQueueVk(DeviceVk &deviceVk);

protected:
	void WaitExecutionFinished() override;

	void Prepare() override;
	void Execute() override;
};

NAMESPACE_END(gr1)