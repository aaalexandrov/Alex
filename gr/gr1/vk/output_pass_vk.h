#pragma once

#include "../output_pass.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;
class PassVk {
	RTTR_ENABLE()
public:
	PassVk(DeviceVk &deviceVk);
	
	void WaitForFences(PassData *passData, DeviceVk *deviceVk);
	void FillWaitSemaphores(PassData *passData, std::vector<vk::Semaphore> &semaphores);

	void AddWaitFence(OutputPass *pass, std::vector<vk::Fence> &fences);
	void AddWaitSemaphore(OutputPass *pass, std::vector<vk::Semaphore> &semaphores);

	vk::UniqueSemaphore _signalSemaphore;
	vk::UniqueFence _signalFence;
};

NAMESPACE_END(gr1)