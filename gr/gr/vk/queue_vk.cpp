#include "queue_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "../graphics_exception.h"
#include "util/mathutl.h"

NAMESPACE_BEGIN(gr)

void QueueVk::Init(DeviceVk &device, int32_t family, int32_t queueIndex, QueueRole role)
{
  _device = &device;
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
    .setLayerCount(std::max(image->_size.w, 1u));

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

void QueueVk::CmdCopyImage(vk::CommandBuffer cmds, ImageVk *dst, util::BoxWithLayer const &dstRegion, uint32_t dstMinMip, uint32_t dstMaxMip, ImageVk *src, glm::uvec4 srcPos, uint32_t srcMinMip)
{
  ASSERT(dstMinMip <= dstMaxMip);
  ASSERT(dstMaxMip < dst->_mipLevels);
  ASSERT(srcMinMip + dstMaxMip - dstMinMip < src->_mipLevels);
  ASSERT(!dstRegion.IsEmpty());
  ASSERT(util::VecLess(dstRegion._max, dst->GetEffectiveSize()));
  ASSERT(util::VecLessEq(srcPos + dstRegion.GetSize(), src->GetEffectiveSize()));

  glm::uvec4 regionSize = dstRegion.GetSize();
  std::vector<vk::ImageCopy> regions;

  vk::ImageAspectFlags srcAspect = src->GetImageAspect();
  vk::ImageAspectFlags dstAspect = dst->GetImageAspect();

  glm::ivec3 srcRegionMin = srcPos;
  glm::ivec3 dstRegionMin = dstRegion._min;
  glm::ivec3 dstRegionMax = dstRegion._max;

  srcRegionMin >>= srcMinMip;
  dstRegionMin >>= dstMinMip;
  dstRegionMax += (1 << dstMinMip) - 1;
  dstRegionMax >>= dstMinMip;

  uint32_t mipLevels = dstMaxMip - dstMinMip;
  for (uint32_t level = 0; level <= mipLevels; ++level) {
    vk::ImageSubresourceLayers srcSubres;
    srcSubres
      .setAspectMask(srcAspect)
      .setMipLevel(srcMinMip + level)
      .setBaseArrayLayer(srcPos.y)
      .setLayerCount(regionSize.y);

    vk::ImageSubresourceLayers dstSubres;
    dstSubres
      .setAspectMask(dstAspect)
      .setMipLevel(dstMinMip + level)
      .setBaseArrayLayer(dstRegion._min.y)
      .setLayerCount(regionSize.y);

    glm::uvec3 extent = dstRegionMax - dstRegionMin + glm::ivec3(1);

    regions.emplace_back(vk::ImageCopy()
      .setSrcSubresource(srcSubres)
      .setSrcOffset({ srcRegionMin.x, srcRegionMin.y, srcRegionMin.z })
      .setDstSubresource(dstSubres)
      .setDstOffset({ dstRegionMin.x, dstRegionMin.y, dstRegionMin.z })
      .setExtent({ extent.x, extent.y, extent.z }));

    srcRegionMin >>= 1;
    dstRegionMin >>= 1;
    dstRegionMax += 1;
    dstRegionMax >>= 1;
  }

  cmds.copyImage(src->_image, src->_layout, dst->_image, dst->_layout, regions);
}

vk::PipelineStageFlags QueueVk::GetPipelineStageFlags(QueueRole role)
{
  vk::PipelineStageFlags flags;
  switch (role) {
    case QueueRole::Graphics:
      flags = vk::PipelineStageFlagBits::eAllGraphics;
      break;
    case QueueRole::Compute:
      flags = vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect;
      break;
    case QueueRole::Transfer:
      flags = vk::PipelineStageFlagBits::eTransfer;
      break;
    case QueueRole::SparseOp:
      flags = vk::PipelineStageFlagBits::eAllCommands; // ??
      break;
    case QueueRole::Present:
      flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      break;
    default:
      throw GraphicsException("Invalid queue role", -1);
      break;
  }
  return flags;
}

vk::AccessFlags QueueVk::GetAccessFlags(QueueRole role)
{
  vk::AccessFlags flags;
  switch (role) {
    case QueueRole::Graphics:
      flags = vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eUniformRead |
        vk::AccessFlagBits::eInputAttachmentRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case QueueRole::Compute:
      flags = vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
      break;
    case QueueRole::Transfer:
      flags = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
      break;
    case QueueRole::SparseOp:
      flags = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryRead; // ??
      break;
    case QueueRole::Present:
      flags = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      break;
    default:
      throw GraphicsException("Invalid queue role", -1);
      break;
  }
  return flags;
}

NAMESPACE_END(gr)