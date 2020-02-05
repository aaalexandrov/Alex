#include "image_transition_pass_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "execution_queue_vk.h"
#include "presentation_surface_vk.h"
#include "rttr/rttr_cast.h"

NAMESPACE_BEGIN(gr1)

ImageTransitionPassVk::ImageTransitionPassVk(Device &device)
	: ResourceStateTransitionPass(device)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	_queueTransitionSemaphore = deviceVk->CreateSemaphore();
}

void ImageTransitionPassVk::Prepare()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	ImageVk *img = static_cast<ImageVk*>(_resource.get());

	ImageVk::StateInfo srcState = img->GetStateInfo(_srcState);
	ImageVk::StateInfo dstState = img->GetStateInfo(_dstState);

	QueueVk *srcQueue, *dstQueue;
	bool queueTransition = GetTransitionQueueInfo(deviceVk, srcState._queueRole, dstState._queueRole, srcQueue, dstQueue);

	_srcCmds = srcQueue->AllocateCmdBuffer();

	_srcCmds->begin(vk::CommandBufferBeginInfo());

	vk::ImageSubresourceRange fullImage;
	fullImage
		.setAspectMask(img->GetImageAspect())
		.setBaseMipLevel(0)
		.setLevelCount(img->GetMipLevels())
		.setBaseArrayLayer(0)
		.setLayerCount(std::max(img->GetArrayLayers(), 1u));
	std::array<vk::ImageMemoryBarrier, 1> imgBarrier;
	imgBarrier[0]
		.setSrcAccessMask(srcState._access)
		.setDstAccessMask(queueTransition ? vk::AccessFlags() : dstState._access)
		.setOldLayout(srcState._layout)
		.setNewLayout(dstState._layout)
		.setSrcQueueFamilyIndex(queueTransition ? srcQueue->_family : VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(queueTransition ? dstQueue->_family : VK_QUEUE_FAMILY_IGNORED)
		.setImage(img->_image)
		.setSubresourceRange(fullImage);
	_srcCmds->pipelineBarrier(srcState._stages, queueTransition ? vk::PipelineStageFlagBits::eBottomOfPipe : dstState._stages,
		vk::DependencyFlags(), nullptr, nullptr, imgBarrier);

	_srcCmds->end();

	if (queueTransition) {

		_dstCmds = dstQueue->AllocateCmdBuffer();

		_dstCmds->begin(vk::CommandBufferBeginInfo());

		imgBarrier[0]
			.setSrcAccessMask(vk::AccessFlags())
			.setDstAccessMask(dstState._access)
			.setOldLayout(dstState._layout)
			.setNewLayout(dstState._layout)
			.setSrcQueueFamilyIndex(srcQueue->_family)
			.setDstQueueFamilyIndex(dstQueue->_family)
			.setImage(img->_image)
			.setSubresourceRange(fullImage);
		_dstCmds->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, dstState._stages,
			vk::DependencyFlags(), nullptr, nullptr, imgBarrier);
		
		_dstCmds->end();
	}
}

void ImageTransitionPassVk::Execute(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	ImageVk *img = static_cast<ImageVk*>(_resource.get());

	ImageVk::StateInfo srcState = img->GetStateInfo(_srcState);
	ImageVk::StateInfo dstState = img->GetStateInfo(_dstState);

	QueueVk *srcQueue, *dstQueue;
	bool queueTransition = GetTransitionQueueInfo(deviceVk, srcState._queueRole, dstState._queueRole, srcQueue, dstQueue);

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);
	std::vector<vk::Semaphore> signalSemaphores;
	FillDependencySemaphores(dependencies, DependencyType::Output, signalSemaphores);

	std::array<vk::SubmitInfo, 1> srcSubmit, dstSubmit; 

	if (_srcState == ResourceState::PresentAcquired) {
		PresentationSurfaceVk *surfaceOwner = rttr::rttr_cast<PresentationSurfaceVk*>(img->_owner);
		waitSemaphores.push_back(*surfaceOwner->_acquireSemaphore);
		waitStageFlags.push_back(srcState._stages);
	}

	if (queueTransition) {

		srcSubmit[0]
			.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_srcCmds)
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&*_queueTransitionSemaphore);
		srcQueue->_queue.submit(srcSubmit, nullptr);


		waitSemaphores.clear();
		waitStageFlags.clear();
		waitSemaphores.push_back(*_queueTransitionSemaphore);
		waitStageFlags.push_back(srcState._stages);

		dstSubmit[0]
			.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_dstCmds)
			.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
			.setPSignalSemaphores(signalSemaphores.data());
		dstQueue->_queue.submit(dstSubmit, nullptr);

	} else {

		srcSubmit[0]
			.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
			.setPWaitSemaphores(waitSemaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data())
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_srcCmds)
			.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
			.setPSignalSemaphores(signalSemaphores.data());
		srcQueue->_queue.submit(srcSubmit, nullptr);

	}
}

vk::PipelineStageFlags ImageTransitionPassVk::GetPassDstStages()
{
	ImageVk *img = static_cast<ImageVk*>(_resource.get());
	ImageVk::StateInfo dstState = img->GetStateInfo(_dstState);
	return dstState._stages;
}


NAMESPACE_END(gr1)

