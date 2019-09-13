#include "image_vk.h"

#include "util/mem.h"
#include "device_vk.h"
#include "physical_device_vk.h"
#include "operation_queue_vk.h"
#include "image_update_vk.h"
#include <algorithm>

NAMESPACE_BEGIN(gr)

util::ValueRemapper<vk::Format, ColorFormat> ImageVk::_vkFormat2ColorFormat { {
  { vk::Format::eUndefined,          ColorFormat::Invalid    },
  { vk::Format::eR8G8B8A8Unorm,      ColorFormat::R8G8B8A8   },
  { vk::Format::eD24UnormS8Uint,     ColorFormat::D24S8      },
} };

ImageVk::ImageVk(DeviceVk &device, vk::Image image, vk::Format format, glm::uvec4 size, uint32_t mipLevels, Usage usage)
  : Image(usage, Vk2ColorFormat(format), size, mipLevels)
  , _device { &device }
  , _layout { vk::ImageLayout::eUndefined }
  , _image { image }
{
  CreateView();
}

ImageVk::ImageVk(DeviceVk &device, vk::Format format, glm::uvec4 size, uint32_t mipLevels, Usage usage)
  : Image(usage, Vk2ColorFormat(format), size, mipLevels)
  , _device { &device }
  , _layout { usage == Usage::Staging ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined }
{
  size = GetEffectiveSize(size);
  vk::ImageCreateInfo imgInfo;
  imgInfo
    .setImageType(GetImageType(_size))
    .setExtent({ size.x, size.y, size.z })
    .setFormat(format)
    .setMipLevels(std::max(mipLevels, 1u))
    .setArrayLayers(size.w)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setUsage(GetImageUsage(usage))
    .setTiling(usage == Usage::Staging ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setInitialLayout(_layout);

  _ownImage = _device->_device->createImageUnique(imgInfo, _device->AllocationCallbacks());
  _image = *_ownImage;

  vk::MemoryRequirements memReq = _device->_device->getImageMemoryRequirements(_image);
  _memory = VmaAllocateMemoryUnique(*_device->_allocator, memReq, usage == Usage::Staging);
  vmaBindImageMemory(*_device->_allocator, *_memory, _image);

  CreateView();
}

util::TypeInfo *ImageVk::GetType()
{
  return util::TypeInfo::Get<ImageVk>();
}

void ImageVk::UpdateContents(util::BoxWithLayer const &region, uint32_t mipLevel, ImageData const &content, glm::uvec4 contentPos)
{
  if (_usage == Usage::Staging) {
    CopyContentsDirect(region, mipLevel, content, contentPos);
    return;
  }

  std::shared_ptr<ImageVk> staging = _device->GetGraphics()->CreateImageTyped<ImageVk>(Usage::Staging, GetColorFormat(), region.GetSize(), 1);
  util::BoxWithLayer stagingRegion { glm::zero<glm::uvec4>(), region.GetSize() - glm::one<glm::uvec4>() };
  staging->CopyContentsDirect(stagingRegion, 0, content, contentPos);

  OperationQueueVk &opQueue = *_device->GetGraphics()->_operationQueue;
  auto updateOp = std::make_unique<ImageUpdateVk>(*this, region, mipLevel, mipLevel, *staging, glm::zero<glm::uvec4>(), 0);
  opQueue.AddOperation(std::move(updateOp));
}

void ImageVk::CopyContentsDirect(util::BoxWithLayer const &region, uint32_t mipLevel, ImageData const &content, glm::uvec4 contentPos)
{
  ASSERT(!region.IsEmpty() && util::VecLess(region._max, GetSize()));
  ASSERT(_usage == Usage::Staging);

  vk::ImageSubresource subRes { GetImageAspect(), mipLevel, 0 };
  vk::SubresourceLayout layout = _device->_device->getImageSubresourceLayout(_image, subRes);

  util::AutoFree<uint8_t*> mem { static_cast<uint8_t*>(Map()), [this](void*) { Unmap(); } };

  glm::uvec4 mappedPitch { GetColorFormatSize(), layout.rowPitch, layout.depthPitch, layout.arrayPitch };
  ImageData mappedData { GetSize(), mappedPitch, mem.Get() + layout.offset };
  ImageData::Copy(content, contentPos, mappedData, region._min, region.GetSize());
}

void *ImageVk::Map()
{
  return VmaMapMemory(*_device->_allocator, *_memory);
}

void ImageVk::Unmap()
{
  vmaUnmapMemory(*_device->_allocator, *_memory);
}

void ImageVk::CreateView()
{
  vk::ImageSubresourceRange range(GetImageAspect(), 0, std::max(_mipLevels, 1u), 0, std::max(_size.w, 1u));
  vk::ImageViewCreateInfo viewInfo;
  viewInfo
    .setImage(_image)
    .setFormat(GetVkFormat())
    .setViewType(GetImageViewType())
    .setComponents({ vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity })
    .setSubresourceRange(range);

  _view = _device->_device->createImageViewUnique(viewInfo, _device->AllocationCallbacks());
}

void ImageVk::RecordTransitionCommands(vk::CommandBuffer srcCommands, QueueVk *srcQueue, vk::CommandBuffer dstCommands, QueueVk *dstQueue, vk::PipelineStageFlags &dstStageFlags)
{
  std::array<vk::ImageMemoryBarrier, 1> srcImgBarrier;
  srcImgBarrier[0]
    .setSrcAccessMask(GetImageAccess() & srcQueue->GetAccessFlags())
    .setOldLayout(GetEffectiveImageLayout(srcQueue))
    .setNewLayout(GetEffectiveImageLayout(dstQueue))
    .setSrcQueueFamilyIndex(srcQueue->_family)
    .setDstQueueFamilyIndex(dstQueue->_family)
    .setImage(_image)
    .setSubresourceRange(vk::ImageSubresourceRange()
      .setAspectMask(GetImageAspect())
      .setBaseArrayLayer(0)
      .setLayerCount(std::max(_size.w, 1u))
      .setBaseMipLevel(0)
      .setLevelCount(_mipLevels));
  ASSERT(srcImgBarrier[0].srcAccessMask);
  auto srcStageFlags = GetImagePipelineStages() & srcQueue->GetPipelineStageFlags();
  ASSERT(srcStageFlags);

  std::array<vk::ImageMemoryBarrier, 1> dstImgBarrier;
  dstImgBarrier[0]
    .setDstAccessMask(GetImageAccess() & dstQueue->GetAccessFlags())
    .setOldLayout(srcImgBarrier[0].newLayout)
    .setNewLayout(srcImgBarrier[0].newLayout)
    .setSrcQueueFamilyIndex(srcQueue->_family)
    .setDstQueueFamilyIndex(dstQueue->_family)
    .setImage(_image)
    .setSubresourceRange(srcImgBarrier[0].subresourceRange);
  ASSERT(dstImgBarrier[0].dstAccessMask);
  dstStageFlags = GetImagePipelineStages() & dstQueue->GetPipelineStageFlags();
  ASSERT(dstStageFlags);

  // we always try to do the transition on the transfer queue, if there's one
  // we've scheduled the layout transition to the first queue, swap that in case the second queue is transfer
  if (dstQueue->_role == QueueVk::Role::Transfer) {
    std::swap(srcImgBarrier[0].oldLayout, dstImgBarrier[0].oldLayout);
    std::swap(srcImgBarrier[0].newLayout, dstImgBarrier[0].newLayout);
  }

  srcCommands.pipelineBarrier(srcStageFlags, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), nullptr, nullptr, srcImgBarrier);

  dstCommands.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, dstStageFlags, vk::DependencyFlags(), nullptr, nullptr, dstImgBarrier);
}

vk::ImageLayout ImageVk::GetEffectiveImageLayout(QueueVk *queue) const
{
  if (_usage != Usage::Staging && queue->_role == QueueVk::Role::Transfer)
    return vk::ImageLayout::eTransferDstOptimal;
  return _layout;
}

vk::ImageViewType ImageVk::GetImageViewType()
{
  ASSERT(_size.x > 0);
  if (!_size.y) {
    if (_size.w)
      return vk::ImageViewType::e1DArray;
    return vk::ImageViewType::e1D;
  } 

  if (!_size.z) {
    if (_size.w)
      return vk::ImageViewType::e2DArray;
    return vk::ImageViewType::e2D;
  }

  return vk::ImageViewType::e3D;
}

vk::ImageType ImageVk::GetImageType(glm::uvec3 const &size)
{
  ASSERT(size.x > 0);
  if (!size.y)
    return vk::ImageType::e1D;
  if (!size.z)
    return vk::ImageType::e2D;
  return vk::ImageType::e3D;
}

vk::ImageUsageFlags ImageVk::GetImageUsage(Usage usage)
{
  switch (usage) {
    case Usage::DepthBuffer:
      return vk::ImageUsageFlagBits::eDepthStencilAttachment;
    case Usage::Staging:
      return vk::ImageUsageFlagBits::eTransferSrc;
    case Usage::Texture:
      return vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
  }
  throw GraphicsException("ImageVk::GetImageUsage(): Unsupported usage value " + static_cast<uint32_t>(usage), VK_RESULT_MAX_ENUM);
}

vk::ImageAspectFlags ImageVk::GetImageAspect(vk::Format format)
{
  switch (format) {
    case vk::Format::eD16Unorm:
    case vk::Format::eD32Sfloat:
    case vk::Format::eX8D24UnormPack32:
      return vk::ImageAspectFlagBits::eDepth;
    case vk::Format::eD16UnormS8Uint:
    case vk::Format::eD24UnormS8Uint:
    case vk::Format::eD32SfloatS8Uint:
      return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    case vk::Format::eS8Uint:
      return vk::ImageAspectFlagBits::eStencil;
    default:
      return vk::ImageAspectFlagBits::eColor;
  }
}

vk::AccessFlags ImageVk::GetImageAccess(Usage usage)
{
  vk::AccessFlags flags;
  switch (usage) {
    case Usage::DepthBuffer:
      flags = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case Usage::Staging:
      flags = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eHostRead | vk::AccessFlagBits::eHostWrite;
      break;
    case Usage::Texture:
      flags = vk::AccessFlagBits::eTransferWrite | vk::AccessFlagBits::eShaderRead;
      break;
    default:
      throw GraphicsException("ImageVk::GetImageAccess(): Unsupported usage value " + static_cast<uint32_t>(usage), VK_RESULT_MAX_ENUM);
  }
  return flags;
}

vk::PipelineStageFlags ImageVk::GetImagePipelineStages(Usage usage)
{
  vk::PipelineStageFlags flags;
  switch (usage) {
    case Usage::DepthBuffer:
      flags = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
      break;
    case Usage::Staging:
      flags = vk::PipelineStageFlagBits::eTransfer;
      break;
    case Usage::Texture:
      flags = vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;
      break;
    default:
      throw GraphicsException("ImageVk::GetImagePipelineStages(): Unsupported usage value " + static_cast<uint32_t>(usage), VK_RESULT_MAX_ENUM);
  }
  return flags;
}

NAMESPACE_END(gr)
