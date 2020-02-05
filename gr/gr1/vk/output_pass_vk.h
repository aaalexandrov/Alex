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
	virtual vk::PipelineStageFlags GetPassDstStages() = 0;

	void FillDependencySemaphores(PassDependencyTracker &dependencies, DependencyType depType, std::vector<vk::Semaphore> &semaphores, std::vector<vk::PipelineStageFlags> *semaphoreWaitStages = nullptr);

	static bool GetTransitionQueueInfo(DeviceVk *deviceVk, QueueRole srcRole, QueueRole dstRole, QueueVk *&srcQueue, QueueVk *&dstQueue);

private:
	vk::UniqueSemaphore _signalSemaphore;
};

NAMESPACE_END(gr1)