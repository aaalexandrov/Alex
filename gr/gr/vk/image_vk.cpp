#include "image_vk.h"

#include "device_vk.h"
#include "physical_device_vk.h"
#include <algorithm>

NAMESPACE_BEGIN(gr)

util::ValueRemapper<vk::Format, ColorFormat> ImageVk::_vkFormat2ColorFormat { {
  { vk::Format::eUndefined,          ColorFormat::Invalid    },
  { vk::Format::eR8G8B8A8Unorm,      ColorFormat::R8G8B8A8   },
  { vk::Format::eD24UnormS8Uint,     ColorFormat::D24S8      },
} };

ImageVk::ImageVk(DeviceVk &device, vk::Image image, vk::Format format, vk::Extent3D size, uint32_t mipLevels, uint32_t arrayLayers, Usage usage)
  : _device { &device }
  , _format { format }
  , _size { size }
  , _mipLevels { mipLevels }
  , _usage { usage }
  , _layout { vk::ImageLayout::eUndefined }
  , _arrayLayers { arrayLayers }
  , _image { image }
{
  CreateView();
}

ImageVk::ImageVk(DeviceVk &device, vk::Format format, vk::Extent3D size, uint32_t mipLevels, uint32_t arrayLayers, Usage usage)
  : _device { &device }
  , _format { format }
  , _size { size }
  , _mipLevels { mipLevels }
  , _usage { usage }
  , _layout { usage == Usage::Staging ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined }
  , _arrayLayers { arrayLayers }
{
  vk::ImageCreateInfo imgInfo;
  imgInfo
    .setImageType(GetImageType(size))
    .setExtent({ size.width, std::max(size.height, 1u), std::max(size.depth, 1u) })
    .setFormat(format)
    .setMipLevels(std::max(mipLevels, 1u))
    .setArrayLayers(std::max(arrayLayers, 1u))
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

util::TypeInfo * ImageVk::GetType()
{
  return util::TypeInfo::Get<ImageVk>();
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
  vk::ImageSubresourceRange range(GetImageAspect(_format), 0, std::max(_mipLevels, 1u), 0, std::max(_arrayLayers, 1u));
  vk::ImageViewCreateInfo viewInfo;
  viewInfo
    .setImage(_image)
    .setFormat(_format)
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
      .setLayerCount(_arrayLayers)
      .setBaseMipLevel(0)
      .setLevelCount(_mipLevels));
  ASSERT(srcImgBarrier[0].srcAccessMask);
  auto srcStageFlags = GetImagePipelineStages() & srcQueue->GetPipelineStageFlags();
  ASSERT(srcStageFlags);
  srcCommands.pipelineBarrier(srcStageFlags, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), nullptr, nullptr, srcImgBarrier);

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
  ASSERT(_size.width > 0);
  if (!_size.height) {
    if (_arrayLayers)
      return vk::ImageViewType::e1DArray;
    return vk::ImageViewType::e1D;
  } 

  if (!_size.depth) {
    if (_arrayLayers)
      return vk::ImageViewType::e2DArray;
    return vk::ImageViewType::e2D;
  }

  return vk::ImageViewType::e3D;
}

vk::ImageType ImageVk::GetImageType(vk::Extent3D const &size)
{
  ASSERT(size.width > 0);
  if (!size.height)
    return vk::ImageType::e1D;
  if (!size.depth)
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