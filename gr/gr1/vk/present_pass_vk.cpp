#include "present_pass_vk.h"
#include "presentation_surface_vk.h"
#include "device_vk.h"
#include "../graphics_exception.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<PresentPassVk>("PresentPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}


PresentPassVk::PresentPassVk(Device &device)
	: PresentPass(device)
	, PassVk(*static_cast<DeviceVk*>(&device))
{
}

void PresentPassVk::Prepare(PassData *passData)
{
}

void PresentPassVk::Execute(PassData *passData)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	PassDataVk *passDataVk = static_cast<PassDataVk*>(passData);
	PresentationSurfaceVk *surfaceVk = static_cast<PresentationSurfaceVk*>(_surface.get());
	
	vk::PresentInfoKHR presentInfo;
	presentInfo
		.setSwapchainCount(1)
		.setPSwapchains(&*surfaceVk->_swapchain)
		.setPImageIndices(&surfaceVk->_currentImageIndex)
		.setPResults(&_presentResult);

	if (passDataVk->_waitSemaphore) {
		presentInfo
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(&passDataVk->_waitSemaphore);
	}

	vk::Result result = deviceVk->PresentQueue()._queue.presentKHR(presentInfo);
	if (result != vk::Result::eSuccess)
		throw GraphicsException("PresentPassVk::Execute() failed!", (uint32_t)result);

	surfaceVk->AcquireNextImage(*_signalSemaphore, *_signalFence);
}

NAMESPACE_END(gr1)

