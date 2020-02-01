#include "image_transition_pass_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "execution_queue_vk.h"

NAMESPACE_BEGIN(gr1)

ImageTransitionPassVk::ImageTransitionPassVk(Device &device)
	: ResourceStateTransitionPass(device)
	, PassVk(*static_cast<DeviceVk*>(&device))
{
}

void ImageTransitionPassVk::Prepare(PassData *passData)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	ImageVk *img = static_cast<ImageVk*>(_resource);

	ImageVk::StateInfo srcState = img->GetStateInfo(_srcState);
	ImageVk::StateInfo dstState = img->GetStateInfo(_dstState);

	QueueVk *srcQueue, *dstQueue;
	bool queueTransition = GetTransitionQueueInfo(srcState._queueRole, dstState._queueRole, srcQueue, dstQueue);

	_srcCmds = srcQueue->AllocateCmdBuffer();

	_srcCmds->begin(vk::CommandBufferBeginInfo());

	vk::ImageSubresourceRange fullImage;
	fullImage
		.setAspectMask(img->GetImageAspect())
		.setBaseMipLevel(0)
		.setLevelCount(img->GetMipLevels())
		.setBaseArrayLayer(0)
		.setLayerCount(img->GetArrayLayers());
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
		_queueTransitionSemaphore = deviceVk->_device->createSemaphoreUnique(vk::SemaphoreCreateInfo(), deviceVk->AllocationCallbacks());

		_dstCmds = dstQueue->AllocateCmdBuffer();

		_dstCmds->begin(vk::CommandBufferBeginInfo());

		imgBarrier[0]
			.setSrcAccessMask(vk::AccessFlags())
			.setDstAccessMask(dstState._access)
			.setOldLayout(srcState._layout)
			.setNewLayout(dstState._layout)
			.setSrcQueueFamilyIndex(srcQueue->_family)
			.setDstQueueFamilyIndex(dstQueue->_family)
			.setImage(img->_image)
			.setSubresourceRange(fullImage);
		_srcCmds->pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, dstState._stages,
			vk::DependencyFlags(), nullptr, nullptr, imgBarrier);
		
		_dstCmds->end();
	}

	_signalSemaphoreStages = dstState._stages;
}

void ImageTransitionPassVk::Execute(PassData *passData)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	ImageVk *img = static_cast<ImageVk*>(_resource);

	ImageVk::StateInfo srcState = img->GetStateInfo(_srcState);
	ImageVk::StateInfo dstState = img->GetStateInfo(_dstState);

	QueueVk *srcQueue, *dstQueue;
	bool queueTransition = GetTransitionQueueInfo(srcState._queueRole, dstState._queueRole, srcQueue, dstQueue);

	std::vector<vk::Semaphore> semaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillWaitSemaphores(passData, semaphores, waitStageFlags);
	std::array<vk::SubmitInfo, 1> srcSubmit, dstSubmit;

	if (queueTransition) {

		srcSubmit[0]
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_srcCmds)
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&*_queueTransitionSemaphore);
		if (semaphores.size()) {
			srcSubmit[0]
				.setWaitSemaphoreCount(static_cast<uint32_t>(semaphores.size()))
				.setPWaitSemaphores(semaphores.data())
				.setPWaitDstStageMask(waitStageFlags.data());
		}
		srcQueue->_queue.submit(srcSubmit, nullptr);


		semaphores.clear();
		waitStageFlags.clear();
		semaphores.push_back(*_queueTransitionSemaphore);
		waitStageFlags.push_back(srcState._stages);

		dstSubmit[0]
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_dstCmds)
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&*_signalSemaphore)
			.setWaitSemaphoreCount(1)
			.setPWaitSemaphores(semaphores.data())
			.setPWaitDstStageMask(waitStageFlags.data());
		dstQueue->_queue.submit(dstSubmit, *_signalFence);

	} else {

		srcSubmit[0]
			.setCommandBufferCount(1)
			.setPCommandBuffers(&*_srcCmds)
			.setSignalSemaphoreCount(1)
			.setPSignalSemaphores(&*_signalSemaphore);
		if (semaphores.size()) {
			srcSubmit[0]
				.setWaitSemaphoreCount(static_cast<uint32_t>(semaphores.size()))
				.setPWaitSemaphores(semaphores.data())
				.setPWaitDstStageMask(waitStageFlags.data());
		}
		srcQueue->_queue.submit(srcSubmit, *_signalFence);

	}
}

bool ImageTransitionPassVk::GetTransitionQueueInfo(QueueRole srcRole, QueueRole dstRole, QueueVk *&srcQueue, QueueVk *&dstQueue)
{
	ASSERT(srcRole != QueueRole::Invalid && dstRole != QueueRole::Invalid);
	ASSERT(!(srcRole == QueueRole::Any && dstRole == QueueRole::Any));
	bool queueTransition = srcRole != dstRole && srcRole != QueueRole::Any && dstRole != QueueRole::Any;

	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	if (queueTransition) {
		srcQueue = &deviceVk->Queue(srcRole);
		dstQueue = &deviceVk->Queue(dstRole);
	} else {
		srcQueue = &deviceVk->Queue(srcRole != QueueRole::Any ? srcRole : dstRole);
		dstQueue = srcQueue;
	}

	return queueTransition;
}

NAMESPACE_END(gr1)

