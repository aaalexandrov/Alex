#include "final_pass_vk.h"
#include "device_vk.h"
#include "../execution_queue.h"
#include "../graphics_exception.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<FinalPassVk>("FinalPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

FinalPassVk::FinalPassVk(Device &device)
	: FinalPass(device)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_passesFinished = deviceVk->CreateFence();

	_cmdFinish = deviceVk->GraphicsQueue()._cmdPool.AllocateCmdBuffer();
	_cmdFinish->begin(vk::CommandBufferBeginInfo());
	_cmdFinish->end();
}

void FinalPassVk::Prepare()
{
}

void FinalPassVk::Execute(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::array<vk::Fence, 1> passFence{ *_passesFinished };
	deviceVk->_device->resetFences(passFence);

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);

	std::array<vk::SubmitInfo, 1> submit;

	submit[0]
		.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStageFlags.data())
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*_cmdFinish);
	deviceVk->GraphicsQueue()._queue.submit(submit, *_passesFinished);
}

bool FinalPassVk::IsFinished()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	vk::Result status = deviceVk->_device->getFenceStatus(*_passesFinished);
	if (status != vk::Result::eSuccess && status != vk::Result::eNotReady)
		throw GraphicsException("Unexpected fence status!", static_cast<uint32_t>(status));
	return status == vk::Result::eSuccess;
}

void FinalPassVk::WaitToFinish()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	
	std::array<vk::Fence, 1> passFence{ *_passesFinished };
	vk::Result result = deviceVk->_device->waitForFences(passFence, true, std::numeric_limits<uint64_t>::max());
	if (result != vk::Result::eSuccess)
		throw GraphicsException("FinalPassVk::WaitToFinish() failed!", (uint32_t)result);
}

NAMESPACE_END(gr1)

