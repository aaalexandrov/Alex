#pragma once

#include "vk.h"
#include "vma.h"
#include "../image.h"
#include "owned_by_queue_vk.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

class ImageVk : public Image, public OwnedByQueueVk<ImageVk> {
public:
  ImageVk(DeviceVk &device, vk::Image image, vk::Format format, glm::uvec4 size, uint32_t mipLevels, Usage usage);
  ImageVk(DeviceVk &device, vk::Format format, glm::uvec4 size, uint32_t mipLevels, Usage usage);

  util::TypeInfo *GetType() override;

  void UpdateContents(util::BoxWithLayer const &region, uint32_t mipLevel, ImageData const &content, glm::uvec4 contentPos) override;

  void *Map() override;
  void Unmap() override;

  void CopyContentsDirect(util::BoxWithLayer const &region, uint32_t mipLevel, ImageData const &content, glm::uvec4 contentPos);

  void CreateView();

  void RecordTransitionCommands(vk::CommandBuffer srcCommands, QueueVk *srcQueue, vk::CommandBuffer dstCommands, QueueVk *dstQueue, vk::PipelineStageFlags &dstStageFlags);

  vk::ImageLayout GetEffectiveImageLayout(QueueVk *queue) const;

  vk::Format GetVkFormat() const { return ColorFormat2Vk(_format); }

  static ColorFormat Vk2ColorFormat(vk::Format vkFmt) { return _vkFormat2ColorFormat.ToDst(vkFmt); }
  static vk::Format ColorFormat2Vk(ColorFormat clrFmt) { return _vkFormat2ColorFormat.ToSrc(clrFmt); }

  vk::ImageViewType GetImageViewType();
  static vk::ImageType GetImageType(glm::uvec3 const &size);
  static vk::ImageUsageFlags GetImageUsage(Usage usage);
  vk::ImageAspectFlags GetImageAspect() const { return GetImageAspect(GetVkFormat()); }
  static vk::ImageAspectFlags GetImageAspect(vk::Format format);
  vk::AccessFlags GetImageAccess() const { return GetImageAccess(_usage); }
  static vk::AccessFlags GetImageAccess(Usage usage);
  vk::PipelineStageFlags GetImagePipelineStages() const { return GetImagePipelineStages(_usage); }
  static vk::PipelineStageFlags GetImagePipelineStages(Usage usage);

  DeviceVk *_device;
  vk::Image _image;
  vk::UniqueImage _ownImage;
  vk::UniqueImageView _view;
  UniqueVmaAllocation _memory;
  vk::ImageLayout _layout;

  static util::ValueRemapper<vk::Format, ColorFormat> _vkFormat2ColorFormat;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::OwnedByQueueVk<gr::ImageVk>)
RTTI_BIND(gr::ImageVk, gr::Image, gr::OwnedByQueueVk<gr::ImageVk>)