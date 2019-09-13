#include "image_update_vk.h"
#include "image_vk.h"
#include "device_vk.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

ImageUpdateVk::ImageUpdateVk(ImageVk &dstImage, util::BoxWithLayer const &dstRegion, uint32_t dstMinMipLevel, uint32_t dstMaxMipLevel, ImageVk &srcImage, glm::uvec4 srcPos, uint32_t srcMinMipLevel)
  : _dstImage(util::SharedFromThis<ImageVk>(&dstImage))
  , _srcImage(util::SharedFromThis<ImageVk>(&srcImage))
  , _dstRegion(dstRegion)
  , _srcPos(srcPos)
  , _dstMinMipLevel(dstMinMipLevel)
  , _dstMaxMipLevel(dstMaxMipLevel)
  , _srcMinMipLevel(srcMinMipLevel)
{
}

void ImageUpdateVk::Prepare(OperationQueue *operationQueue)
{
  DeviceVk *device = _dstImage->_device;
  ASSERT(!_transitionToTransfer);
  _transitionToTransfer = _dstImage->GetQueueTransitionTo(&device->_transferQueue);

  _transferCmds = device->_transferQueue.AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  _transferCmds->begin(beginInfo);

  QueueVk::CmdSetImageLayout(_transferCmds.get(), _srcImage.get(), vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead);

  QueueVk::CmdCopyImage(_transferCmds.get(), _dstImage.get(), _dstRegion, _dstMinMipLevel, _dstMaxMipLevel, _srcImage.get(), _srcPos, _srcMinMipLevel);

  QueueVk::CmdSetImageLayout(_transferCmds.get(), _srcImage.get(), vk::ImageLayout::eGeneral, vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eHostWrite);

  _transferCmds->end();
}

void ImageUpdateVk::Execute(OperationQueue *operationQueue)
{
  if (_transitionToTransfer)
    _transitionToTransfer->ExecuteQueueTransition(operationQueue);

  DeviceVk *device = _dstImage->_device;
  std::array<vk::SubmitInfo, 1> transferSubmit;
  transferSubmit[0]
    .setCommandBufferCount(1)
    .setPCommandBuffers(&_transferCmds.get());

  device->_transferQueue._queue.submit(transferSubmit, nullptr);
}

NAMESPACE_END(gr)

