#include "image_update_vk.h"
#include "image_vk.h"
#include "device_vk.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

ImageUpdateVk::ImageUpdateVk(ImageVk &updatedImage, ImageVk &sourceImage)
  : _image(util::SharedFromThis<ImageVk>(&updatedImage))
{
}

void ImageUpdateVk::Prepare(OperationQueue *operationQueue)
{
  DeviceVk *device = _image->_device;
  ASSERT(!_transitionToTransfer);
  _transitionToTransfer = _image->GetQueueTransitionTo(&device->_transferQueue);

  _transferCmds = device->_transferQueue.AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  _transferCmds->begin(beginInfo);

  ASSERT(_stagingImage->_layout == vk::ImageLayout::eUndefined);
  QueueVk::CmdSetImageLayout(_transferCmds.get(), _stagingImage.get(), vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead);
  QueueVk::CmdSetImageLayout(_transferCmds.get(), _image.get(), vk::ImageLayout::eTransferDstOptimal, vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite);

  QueueVk::CmdCopyImage(_transferCmds.get(), _stagingImage.get(), _image.get());

  QueueVk::CmdSetImageLayout(_transferCmds.get(), _image.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
  QueueVk::CmdSetImageLayout(_transferCmds.get(), _stagingImage.get(), vk::ImageLayout::eGeneral, vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eHostWrite);

  _transferCmds->end();
}

void ImageUpdateVk::Execute(OperationQueue *operationQueue)
{
  if (_transitionToTransfer)
    _transitionToTransfer->ExecuteQueueTransition(operationQueue);


}

NAMESPACE_END(gr)

