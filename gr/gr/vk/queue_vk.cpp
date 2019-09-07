#include "queue_vk.h"
#include "device_vk.h"
#include "image_vk.h"

NAMESPACE_BEGIN(gr)

void QueueVk::Init(DeviceVk &device, int32_t family, int32_t queueIndex)
{
  _family = family;
  _queue = device._device->getQueue(family, queueIndex);

  vk::CommandPoolCreateInfo poolInfo { vk::CommandPoolCreateFlagBits::eResetCommandBuffer, static_cast<uint32_t>(family) };
  _cmdPool = device._device->createCommandPoolUnique(poolInfo);
}

vk::UniqueCommandBuffer &&QueueVk::AllocateCmdBuffer()
{
  vk::CommandBufferAllocateInfo cmdsInfo(*_cmdPool, vk::CommandBufferLevel::ePrimary, 1);
  std::vector<vk::UniqueCommandBuffer> buffers = _device->_device->allocateCommandBuffersUnique(cmdsInfo);
  return std::move(buffers[0]);
}

void QueueVk::CmdSetImageLayout(vk::CommandBuffer cmds, ImageVk *image, vk::ImageLayout layout, vk::AccessFlags priorAccess, vk::AccessFlags followingAccess)
{
  vk::ImageSubresourceRange imgSubresourceRange;
  imgSubresourceRange
    .setAspectMask(ImageVk::GetImageAspect(image->_format))
    .setBaseMipLevel(0)
    .setLevelCount(image->_mipLevels)
    .setBaseArrayLayer(0)
    .setLayerCount(image->_arrayLayers);

  std::array<vk::ImageMemoryBarrier, 1> imgBarriers;
  imgBarriers[0]
    .setSrcAccessMask(priorAccess)
    .setDstAccessMask(followingAccess)
    .setOldLayout(image->_layout)
    .setNewLayout(layout)
    .setImage(image->_image)
    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
    .setSubresourceRange(imgSubresourceRange);

  cmds.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), nullptr, nullptr, imgBarriers);

  image->_layout = layout;

}

void QueueVk::CmdCopyImage(vk::CommandBuffer cmds, ImageVk *src, ImageVk *dst)
{
  uint32_t layers = std::min(src->_arrayLayers, dst->_arrayLayers);
  vk::ImageSubresourceLayers srcSubres;
  srcSubres
    .setAspectMask(ImageVk::GetImageAspect(src->_format))
    .setMipLevel(0)
    .setBaseArrayLayer(0)
    .setLayerCount(layers);

  vk::ImageSubresourceLayers dstSubres;
  dstSubres
    .setAspectMask(ImageVk::GetImageAspect(dst->_format))
    .setMipLevel(0)
    .setBaseArrayLayer(0)
    .setLayerCount(layers);

  std::array<vk::ImageCopy, 1> regions;
  regions[0]
    .setSrcSubresource(srcSubres)
    .setSrcOffset({ 0, 0, 0 })
    .setDstSubresource(dstSubres)
    .setDstOffset({ 0, 0, 0 })
    .setExtent({ std::min(src->_size.width, dst->_size.width), std::min(src->_size.height, dst->_size.height), std::min(src->_size.depth, dst->_size.depth) });

  cmds.copyImage(src->_image, src->_layout, dst->_image, dst->_layout, regions);
}

NAMESPACE_END(gr)