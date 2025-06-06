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
{
}

void PresentPassVk::Prepare()
{
}

void PresentPassVk::Submit(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	PresentationSurfaceVk *surfaceVk = static_cast<PresentationSurfaceVk*>(_surface.get());

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);
	std::vector<vk::Semaphore> signalSemaphores;
	FillDependencySemaphores(dependencies, DependencyType::Output, signalSemaphores);

	uint32_t imageIndex = surfaceVk->GetImageIndex(_surfaceImage);
	ASSERT(imageIndex != ~0);
	vk::PresentInfoKHR presentInfo;
	presentInfo
		.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
		.setPWaitSemaphores(waitSemaphores.data())
		.setSwapchainCount(1)
		.setPSwapchains(&*surfaceVk->_swapchain)
		.setPImageIndices(&imageIndex)
		.setPResults(&_presentResult);
	vk::Result result;
	try {
		result = deviceVk->PresentQueue()._queue.presentKHR(presentInfo);
	} catch (vk::OutOfDateKHRError e) {
		result = (vk::Result)e.code().value();
	} catch (vk::SurfaceLostKHRError e) {
		result = (vk::Result)e.code().value();
	}
	// except for success, in addition to the above errors result could also be vk::Result::eSuboptimalKHR

	std::array<vk::SubmitInfo, 1> submit;
	submit[0]
		.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
		.setPSignalSemaphores(signalSemaphores.data());
	deviceVk->PresentQueue()._queue.submit(submit, nullptr);
}

NAMESPACE_END(gr1)

