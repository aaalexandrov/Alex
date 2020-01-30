#include "image_vk.h"

#include "device_vk.h"
#include "image_transition_pass_vk.h"
#include "../execution_queue.h"
#include "../graphics_exception.h"
#include "util/mem.h"
#include "rttr/registration.h"
#include <algorithm>

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<ImageVk>("ImageVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

util::ValueRemapper<vk::Format, ColorFormat> ImageVk::s_vkFormat2ColorFormat { {
		{ vk::Format::eUndefined,          ColorFormat::Invalid    },
		{ vk::Format::eR8G8B8A8Unorm,      ColorFormat::R8G8B8A8   },
		{ vk::Format::eB8G8R8A8Unorm,      ColorFormat::B8G8R8A8   },
		{ vk::Format::eD24UnormS8Uint,     ColorFormat::D24S8      },
	} };

void ImageVk::Init(Usage usage, vk::Image image, vk::Format format, glm::uvec4 size, uint32_t mipLevels)
{
	Image::Init(usage, Vk2ColorFormat(format), size, mipLevels);
	_layout = vk::ImageLayout::eUndefined;
	_image = image;
  CreateView();
}

std::shared_ptr<ResourceStateTransitionPass> ImageVk::CreateTransitionPass(ResourceState srcState, ResourceState dstState)
{
	auto imgTransition = std::make_shared<ImageTransitionPassVk>(*_device);
	imgTransition->Init(*this, srcState, dstState);
	return std::move(imgTransition);
}

void ImageVk::Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels)
{
	Image::Init(usage, format, size, mipLevels);
	_layout = usage == Usage::Staging ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined;

  size = GetEffectiveSize(size);
  vk::ImageCreateInfo imgInfo;
  imgInfo
    .setImageType(GetImageType(_size))
    .setExtent({ size.x, size.y, size.z })
    .setFormat(ColorFormat2Vk(format))
    .setMipLevels(std::max(mipLevels, 1u))
    .setArrayLayers(size.w)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setUsage(GetImageUsage(usage))
    .setTiling(usage == Usage::Staging ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal)
    .setSharingMode(vk::SharingMode::eExclusive)
    .setInitialLayout(_layout);

	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_ownImage = deviceVk->_device->createImageUnique(imgInfo, deviceVk->AllocationCallbacks());
  _image = *_ownImage;

  vk::MemoryRequirements memReq = deviceVk->_device->getImageMemoryRequirements(_image);
  _memory = VmaAllocateMemoryUnique(*deviceVk->_allocator, memReq, usage == Usage::Staging);
  vmaBindImageMemory(*deviceVk->_allocator, *_memory, _image);

  CreateView();
}

void *ImageVk::Map()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
  return VmaMapMemory(*deviceVk->_allocator, *_memory);
}

void ImageVk::Unmap()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	vmaUnmapMemory(*deviceVk->_allocator, *_memory);
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

	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_view = deviceVk->_device->createImageViewUnique(viewInfo, deviceVk->AllocationCallbacks());
}

vk::ImageLayout ImageVk::GetEffectiveImageLayout(QueueVk *queue) const
{
  if (_usage != Usage::Staging && queue->_role == QueueRole::Transfer)
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
	vk::ImageUsageFlags imgUsage;
	if ((usage & Usage::RenderTarget) == Usage::RenderTarget)
		imgUsage |= vk::ImageUsageFlagBits::eColorAttachment;
  switch (usage & ~(Usage::RenderTarget)) {
    case Usage::DepthBuffer:
      imgUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    case Usage::Staging:
      imgUsage |= vk::ImageUsageFlagBits::eTransferSrc;
    case Usage::Texture:
      imgUsage |= vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
  }
	if (imgUsage)
		return imgUsage;
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

NAMESPACE_END(gr1)
