#include "output_pass_vk.h"
#include "device_vk.h"
#include "execution_queue_vk.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

void PassVk::FillDependencySemaphores(PassDependencyTracker &dependencies, DependencyType depType, 
	std::vector<vk::Semaphore> &semaphores, std::vector<vk::PipelineStageFlags> *semaphoreWaitStages)
{
	ASSERT(depType == DependencyType::Input || depType == DependencyType::Output);
	ASSERT((depType == DependencyType::Input) == (bool)semaphoreWaitStages);
	ASSERT(!semaphoreWaitStages || semaphores.size() == semaphoreWaitStages->size());

	OutputPass *thisPass = rttr::rttr_cast<OutputPass*>(this);
	ASSERT(thisPass);

	auto enumDependencies = [&](PassDependency *dependency) 
	{
		PassDependencyVk *depVk = static_cast<PassDependencyVk*>(dependency);
		semaphores.push_back(*depVk->_semaphore);
		if (depType == DependencyType::Input) {
			ASSERT(dependency->_dstPass == thisPass);
			PassVk *passVk = rttr::rttr_cast<PassVk *>(dependency->_srcPass);
			semaphoreWaitStages->push_back(passVk->GetPassDstStages());
		} else {
			ASSERT(dependency->_srcPass == thisPass);
		}
	};

	dependencies.ForDependencies(thisPass, depType, enumDependencies);
}

bool PassVk::GetTransitionQueueInfo(DeviceVk *deviceVk, QueueRole srcRole, QueueRole dstRole, QueueVk *&srcQueue, QueueVk *&dstQueue)
{
	ASSERT(srcRole != QueueRole::Invalid && dstRole != QueueRole::Invalid);
	ASSERT(!(srcRole == QueueRole::Any && dstRole == QueueRole::Any));
	bool roleTransition = srcRole != dstRole && srcRole != QueueRole::Any && dstRole != QueueRole::Any;

	if (roleTransition) {
		srcQueue = &deviceVk->Queue(srcRole);
		dstQueue = &deviceVk->Queue(dstRole);
	} else {
		srcQueue = &deviceVk->Queue(srcRole != QueueRole::Any ? srcRole : dstRole);
		dstQueue = srcQueue;
	}

	return srcQueue->_family != dstQueue->_family;
}


NAMESPACE_END(gr1)

