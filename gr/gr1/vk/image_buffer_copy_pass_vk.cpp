#include "image_buffer_copy_pass_vk.h"
#include "device_vk.h"
#include "buffer_vk.h"
#include "image_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<ImageBufferCopyPassVk>("ImageBufferCopyPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

ImageBufferCopyPassVk::ImageBufferCopyPassVk(Device &device)
	: ImageBufferCopyPass(device)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_cmdCopy = deviceVk->TransferQueue().AllocateCmdBuffer();
}

void ImageBufferCopyPassVk::Prepare()
{
	BufferVk *bufferVk = static_cast<BufferVk*>(_buffer.get());
	ImageVk *imageVk = static_cast<ImageVk*>(_image.get());
	ImageVk::StateInfo imgStateInfo = imageVk->GetStateInfo(GetImageState());

	_cmdCopy->reset(vk::CommandBufferResetFlags());

	_cmdCopy->begin(vk::CommandBufferBeginInfo());

	glm::uvec4 bufferDim = _bufferImageData.GetEffectiveSize();
	glm::uvec4 effectiveSize = ImageData::GetEffectiveSize(_size);
	std::array<vk::BufferImageCopy, 1> copy;
	copy[0]
		.setBufferOffset(_bufferImageData.GetOffset(_bufferOffset))
		.setBufferRowLength(bufferDim.y)
		.setBufferImageHeight(bufferDim.z)
		.setImageSubresource(vk::ImageSubresourceLayers(imageVk->GetImageAspect(), _imageMipLevel, _imageOffset.w, effectiveSize.w))
		.setImageOffset(vk::Offset3D(_imageOffset.x, _imageOffset.y, _imageOffset.z))
		.setImageExtent(vk::Extent3D(effectiveSize.x, effectiveSize.y, effectiveSize.z));

	if (_direction == CopyDirection::BufferToImage) {
		_cmdCopy->copyBufferToImage(*bufferVk->_buffer, imageVk->_image, imgStateInfo._layout, copy);
	} else {
		ASSERT(_direction == CopyDirection::ImageToBuffer);
		_cmdCopy->copyImageToBuffer(imageVk->_image, imgStateInfo._layout, *bufferVk->_buffer, copy);
	}

	_cmdCopy->end();
}

void ImageBufferCopyPassVk::Execute(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);
	std::vector<vk::Semaphore> signalSemaphores;
	FillDependencySemaphores(dependencies, DependencyType::Output, signalSemaphores);

	std::array<vk::SubmitInfo, 1> copySubmit;
	copySubmit[0]
		.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStageFlags.data())
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*_cmdCopy)
		.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
		.setPSignalSemaphores(signalSemaphores.data());
	deviceVk->TransferQueue()._queue.submit(copySubmit, nullptr);
}

NAMESPACE_END(gr1)

