#pragma once

#include "device_vk.h"

NAMESPACE_BEGIN(gr1)

class ExecutionQueueVk : public ExecutionQueue {
	RTTR_ENABLE(ExecutionQueue)
public:
	ExecutionQueueVk(DeviceVk &deviceVk);

protected:
	void WaitExecutionFinished() override;
};

NAMESPACE_END(gr1)