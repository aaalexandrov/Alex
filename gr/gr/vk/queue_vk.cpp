#include "queue_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "../graphics_exception.h"

NAMESPACE_BEGIN(gr)

void QueueVk::Init(DeviceVk &device, int32_t family, int32_t queueIndex, Role role)
{
  _family = family;
  _queue = device._device->getQueue(family, queueIndex);
  _role = role;

  vk::CommandPoolCreateInfo poolInfo { vk::CommandPoolCreateFlagBits::eResetCommandBuffer, static_cast<uint32_t>(family) };
  _cmdPool = device._device->createCommandPoolUnique(poolInfo, device.AllocationCallbacks());
}

vk::UniqueCommandBuffer QueueVk::AllocateCmdBuffer(vk::CommandBufferLevel level)
{
  vk::CommandBufferAllocateInfo cmdsInfo(*_cmdPool, level, 1);
  std::vector<vk::UniqueCommandBuffer> buffers = _device->_device->allocateCommandBuffersUnique(cmdsInfo);
  return std::move(buffers[0]);
}

void QueueVk::CmdSetImageLayout(vk::CommandBuffer cmds, ImageVk *image, vk::ImageLayout layout, vk::AccessFlags priorAccess, vk::AccessFlags followingAccess)
{
  vk::ImageSubresourceRange imgSubresourceRange;
  imgSubresourceRange
    .setAspectMask(image->GetImageAspect())
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
  uint32_t mipLevels = std::min(src->_mipLevels, dst->_mipLevels);
  glm::uvec3 extent { std::min(src->_size.x, dst->_size.x), std::min(src->_size.y, dst->_size.y), std::min(src->_size.z, dst->_size.z) };
  std::vector<vk::ImageCopy> regions;

  vk::ImageAspectFlags srcAspect = src->GetImageAspect();
  vk::ImageAspectFlags dstAspect = dst->GetImageAspect();

  for (uint32_t level = 0; level < mipLevels; ++level) {
    vk::ImageSubresourceLayers srcSubres;
    srcSubres
      .setAspectMask(srcAspect)
      .setMipLevel(level)
      .setBaseArrayLayer(0)
      .setLayerCount(layers);

    vk::ImageSubresourceLayers dstSubres;
    dstSubres
      .setAspectMask(dstAspect)
      .setMipLevel(level)
      .setBaseArrayLayer(0)
      .setLayerCount(layers);

    regions.emplace_back(vk::ImageCopy()
      .setSrcSubresource(srcSubres)
      .setSrcOffset({ 0, 0, 0 })
      .setDstSubresource(dstSubres)
      .setDstOffset({ 0, 0, 0 })
      .setExtent({ extent.x, extent.y, extent.z }));

    extent /= 2;
  }

  cmds.copyImage(src->_image, src->_layout, dst->_image, dst->_layout, regions);
}

vk::PipelineStageFlags QueueVk::GetPipelineStageFlags(Role role)
{
  vk::PipelineStageFlags flags;
  switch (role) {
    case Role::Graphics:
      flags = vk::PipelineStageFlagBits::eAllGraphics;
      break;
    case Role::Compute:
      flags = vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect;
      break;
    case Role::Transfer:
      flags = vk::PipelineStageFlagBits::eTransfer;
      break;
    case Role::SparseOp:
      flags = vk::PipelineStageFlagBits::eAllCommands; // ??
      break;
    case Role::Present:
      flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      break;
    default:
      throw GraphicsException("Invalid queue role", -1);
      break;
  }
  return flags;
}

vk::AccessFlags QueueVk::GetAccessFlags(Role role)
{
  vk::AccessFlags flags;
  switch (role) {
    case Role::Graphics:
      flags = vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eUniformRead |
        vk::AccessFlagBits::eInputAttachmentRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case Role::Compute:
      flags = vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
      break;
    case Role::Transfer:
      flags = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
      break;
    case Role::SparseOp:
      flags = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryRead; // ??
      break;
    case Role::Present:
      flags = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      break;
    default:
      throw GraphicsException("Invalid queue role", -1);
      break;
  }
  return flags;
}

NAMESPACE_END(gr)