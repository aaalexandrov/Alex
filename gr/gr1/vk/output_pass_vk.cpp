#include "output_pass_vk.h"
#include "device_vk.h"

NAMESPACE_BEGIN(gr1)

PassVk::PassVk(DeviceVk &deviceVk)
	: _signalSemaphore(std::move(deviceVk.CreateSemaphore()))
	, _signalFence(std::move(deviceVk.CreateFence()))
{
}

NAMESPACE_END(gr1)