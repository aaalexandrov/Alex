#include "present_pass_vk.h"
#include "presentation_surface_vk.h"
#include "device_vk.h"
#include "execution_queue_vk.h"
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
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_beforePresent = deviceVk->CreateSemaphore();

	_cmdSignal = deviceVk->PresentQueue().AllocateCmdBuffer();
	_cmdSignal->begin(vk::CommandBufferBeginInfo());
	_cmdSignal->end();

	_signalSemaphoreStages = vk::PipelineStageFlagBits::eTransfer;
}

void PresentPassVk::Prepare(PassData *passData)
{
}

void PresentPassVk::Execute(PassData *passData)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	PresentationSurfaceVk *surfaceVk = static_cast<PresentationSurfaceVk*>(_surface.get());

	std::vector<vk::Semaphore> waitSems;
	std::vector<vk::PipelineStageFlags> semWaitStages;
	FillWaitSemaphores(passData, waitSems, semWaitStages);

	std::array<vk::Semaphore, 2> signalSems{ *_beforePresent, *_signalSemaphore };
	std::array<vk::SubmitInfo, 1> submit;
	submit[0]
		.setWaitSemaphoreCount(static_cast<uint32_t>(waitSems.size()))
		.setPWaitSemaphores(waitSems.data())
		.setPWaitDstStageMask(semWaitStages.data())
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*_cmdSignal)
		.setSignalSemaphoreCount(static_cast<uint32_t>(signalSems.size()))
		.setPSignalSemaphores(signalSems.data());
	deviceVk->PresentQueue()._queue.submit(submit, nullptr);

	uint32_t imageIndex = surfaceVk->GetImageIndex(_surfaceImage);
	ASSERT(imageIndex != ~0);
	vk::PresentInfoKHR presentInfo;
	presentInfo
		.setSwapchainCount(1)
		.setPSwapchains(&*surfaceVk->_swapchain)
		.setPImageIndices(&imageIndex)
		.setPResults(&_presentResult)
		.setWaitSemaphoreCount(static_cast<uint32_t>(1))
		.setPWaitSemaphores(&*_beforePresent);
	vk::Result result = deviceVk->PresentQueue()._queue.presentKHR(presentInfo);
	if (result != vk::Result::eSuccess)
		throw GraphicsException("PresentPassVk::Execute() failed!", (uint32_t)result);
}

NAMESPACE_END(gr1)

