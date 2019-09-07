#include "image_update_vk.h"
#include "image_vk.h"
#include "device_vk.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

ImageUpdateVk::ImageUpdateVk(ImageVk &updatedImage, ImageVk &sourceImage)
  : _image(util::SharedFromThis<ImageVk>(&updatedImage))
{
}

void ImageUpdateVk::Prepare()
{
  DeviceVk *device = _image->_device;
  _queue = &device->_transferQueue;
  _semaphoreDone = device->CreateSemaphore();

  _commands = _queue->AllocateCmdBuffer();

  vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr);
  _commands->begin(beginInfo);

  if (_stagingImage->_layout != vk::ImageLayout::eGeneral)
    QueueVk::CmdSetImageLayout(_commands.get(), _stagingImage.get(), vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead);
  QueueVk::CmdSetImageLayout(_commands.get(), _image.get(), vk::ImageLayout::eTransferDstOptimal, vk::AccessFlags(), vk::AccessFlagBits::eTransferWrite);

  QueueVk::CmdCopyImage(_commands.get(), _stagingImage.get(), _image.get());

  QueueVk::CmdSetImageLayout(_commands.get(), _image.get(), vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
  QueueVk::CmdSetImageLayout(_commands.get(), _stagingImage.get(), vk::ImageLayout::eGeneral, vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eHostWrite);

  _commands->end();
}

NAMESPACE_END(gr)

