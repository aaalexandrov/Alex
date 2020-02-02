#include "output_pass_vk.h"
#include "device_vk.h"
#include "../execution_queue.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

PassVk::PassVk(DeviceVk &deviceVk)
	: _signalSemaphore(std::move(deviceVk.CreateSemaphore()))
{
}

void PassVk::FillWaitSemaphores(PassData *passData, std::vector<vk::Semaphore> &semaphores, std::vector<vk::PipelineStageFlags> &semaphoreWaitStages)
{
	ASSERT(semaphores.size() == semaphoreWaitStages.size());
	if (passData->_transitionPasses.size()) {
		for (auto &transition : passData->_transitionPasses) {
			ASSERT(!transition->_transitionPasses.size());
			AddWaitSemaphore(transition->_pass.get(), semaphores, semaphoreWaitStages);
		}
	} else {
		if (passData->_previousPassData) {
			AddWaitSemaphore(passData->_previousPassData->_pass.get(), semaphores, semaphoreWaitStages);
		}
	}
}

void PassVk::AddWaitSemaphore(OutputPass *pass, std::vector<vk::Semaphore>& semaphores, std::vector<vk::PipelineStageFlags> &semaphoreWaitStages)
{
	PassVk *passVk = rttr::rttr_cast<PassVk *>(pass);
	if (passVk->_signalSemaphore) {
		semaphores.push_back(*passVk->_signalSemaphore);
		semaphoreWaitStages.push_back(passVk->_signalSemaphoreStages);
	}
}

bool PassVk::GetTransitionQueueInfo(DeviceVk *deviceVk, QueueRole srcRole, QueueRole dstRole, QueueVk *&srcQueue, QueueVk *&dstQueue)
{
	ASSERT(srcRole != QueueRole::Invalid && dstRole != QueueRole::Invalid);
	ASSERT(!(srcRole == QueueRole::Any && dstRole == QueueRole::Any));
	bool queueTransition = srcRole != dstRole && srcRole != QueueRole::Any && dstRole != QueueRole::Any;

	if (queueTransition) {
		srcQueue = &deviceVk->Queue(srcRole);
		dstQueue = &deviceVk->Queue(dstRole);
	} else {
		srcQueue = &deviceVk->Queue(srcRole != QueueRole::Any ? srcRole : dstRole);
		dstQueue = srcQueue;
	}

	return queueTransition;
}


NAMESPACE_END(gr1)

