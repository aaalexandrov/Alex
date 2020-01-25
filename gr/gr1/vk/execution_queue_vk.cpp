#include "execution_queue_vk.h"

NAMESPACE_BEGIN(gr1)

ExecutionQueueVk::ExecutionQueueVk(DeviceVk &deviceVk)
	: ExecutionQueue(deviceVk)
{

}

void ExecutionQueueVk::WaitExecutionFinished()
{
}


NAMESPACE_END(gr1)

