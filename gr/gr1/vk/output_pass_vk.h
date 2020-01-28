#pragma once

#include "../output_pass.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class PassDataVk : public PassData {
	RTTR_ENABLE(PassData)
public:
	vk::Semaphore _waitSemaphore;
	vk::Fence _waitFence;
};

class DeviceVk;
class PassVk {
	RTTR_ENABLE()
public:
	PassVk(DeviceVk &deviceVk);
	
	vk::UniqueSemaphore _signalSemaphore;
	vk::UniqueFence _signalFence;
};

NAMESPACE_END(gr1)