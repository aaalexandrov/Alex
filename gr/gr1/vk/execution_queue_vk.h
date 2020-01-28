#pragma once

#include "device_vk.h"
#include "output_pass_vk.h"

NAMESPACE_BEGIN(gr1)

class ExecutionQueueVk : public ExecutionQueue {
	RTTR_ENABLE(ExecutionQueue)
public:
	ExecutionQueueVk(DeviceVk &deviceVk);

	void ExecutePasses() override;

protected:
	void WaitExecutionFinished() override;

	void PreparePassData();

	void Prepare();
	void Execute();

	std::vector<PassDataVk> _passData;
};

NAMESPACE_END(gr1)