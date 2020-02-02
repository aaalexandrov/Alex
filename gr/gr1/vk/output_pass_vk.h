#pragma once

#include "../output_pass.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;
enum class QueueRole;
struct QueueVk;

class PassVk {
	RTTR_ENABLE()
public:
	PassVk(DeviceVk &deviceVk);
	
	void FillWaitSemaphores(PassData *passData, std::vector<vk::Semaphore> &semaphores, std::vector<vk::PipelineStageFlags> &semaphoreWaitStages);

	void AddWaitSemaphore(OutputPass *pass, std::vector<vk::Semaphore> &semaphores, std::vector<vk::PipelineStageFlags> &semaphoreWaitStages);

	static bool GetTransitionQueueInfo(DeviceVk *deviceVk, QueueRole srcRole, QueueRole dstRole, QueueVk *&srcQueue, QueueVk *&dstQueue);

	vk::UniqueSemaphore _signalSemaphore;
	vk::PipelineStageFlags _signalSemaphoreStages;
};

NAMESPACE_END(gr1)