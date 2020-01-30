#include "image_transition_pass_vk.h"
#include "device_vk.h"
#include "../execution_queue.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<ImageTransitionPassVk>("ImageTransitionPassVk")
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}


ImageTransitionPassVk::ImageTransitionPassVk(Device &device)
	: ResourceStateTransitionPass(device)
	, PassVk(*static_cast<DeviceVk*>(&device))
{
}

void ImageTransitionPassVk::Prepare(PassData *passData)
{
}

void ImageTransitionPassVk::Execute(PassData *passData)
{
}


NAMESPACE_END(gr1)

