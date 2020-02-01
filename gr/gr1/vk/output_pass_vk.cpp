#include "output_pass_vk.h"
#include "device_vk.h"
#include "../execution_queue.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

PassVk::PassVk(DeviceVk &deviceVk)
	: _signalSemaphore(std::move(deviceVk.CreateSemaphore()))
	, _signalFence(std::move(deviceVk.CreateFence()))
{
}

void PassVk::WaitForFences(PassData *passData, DeviceVk *deviceVk)
{
	std::vector<vk::Fence> fences;
	if (passData->_transitionPasses.size()) {
		for (auto &transition : passData->_transitionPasses) {
			ASSERT(!transition->_transitionPasses.size());
			AddWaitFence(transition->_pass.get(), fences);
		}
	} else {
		if (passData->_previousPassData) {
			AddWaitFence(passData->_previousPassData->_pass.get(), fences);
		}
	}
	if (fences.size()) {
		deviceVk->_device->waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
	}
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

void PassVk::AddWaitFence(OutputPass *pass, std::vector<vk::Fence>& fences)
{
	PassVk *passVk = rttr::rttr_cast<PassVk *>(pass);
	if (passVk->_signalFence) {
		fences.push_back(*passVk->_signalFence);
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

NAMESPACE_END(gr1)

