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
  ImageVk(DeviceVk &device, vk::Image image, vk::Format format, vk::Extent3D size, uint32_t mipLevels, uint32_t arrayLayers, Usage usage);
  ImageVk(DeviceVk &device, vk::Format format, vk::Extent3D size, uint32_t mipLevels, uint32_t arrayLayers, Usage usage);

  util::TypeInfo *GetType() override;

  ColorFormat GetColorFormat() override { return Vk2ColorFormat(_format); }
  Usage GetUsage() override { return _usage; }
  glm::u32vec3 GetSize() override { return glm::u32vec3(_size.width, _size.height, _size.depth); }

  void *Map() override;
  void Unmap() override;

  void CreateView();

  void RecordTransitionCommands(vk::CommandBuffer srcCommands, QueueVk *srcQueue, vk::CommandBuffer dstCommands, QueueVk *dstQueue, vk::PipelineStageFlags &dstStageFlags);

  vk::ImageLayout GetEffectiveImageLayout(QueueVk *queue) const;

  static ColorFormat Vk2ColorFormat(vk::Format vkFmt) { return _vkFormat2ColorFormat.ToDst(vkFmt); }
  static vk::Format ColorFormat2vk(ColorFormat clrFmt) { return _vkFormat2ColorFormat.ToSrc(clrFmt); }

  vk::ImageViewType GetImageViewType();
  static vk::ImageType GetImageType(vk::Extent3D const &size);
  static vk::ImageUsageFlags GetImageUsage(Usage usage);
  vk::ImageAspectFlags GetImageAspect() const { return GetImageAspect(_format); }
  static vk::ImageAspectFlags GetImageAspect(vk::Format format);
  vk::AccessFlags GetImageAccess() const { return GetImageAccess(_usage); }
  static vk::AccessFlags GetImageAccess(Usage usage);
  vk::PipelineStageFlags GetImagePipelineStages() const { return GetImagePipelineStages(_usage); }
  static vk::PipelineStageFlags GetImagePipelineStages(Usage usage);

  Usage _usage;
  DeviceVk *_device;
  vk::Image _image;
  vk::UniqueImage _ownImage;
  vk::UniqueImageView _view;
  UniqueVmaAllocation _memory;
  vk::ImageLayout _layout;
  vk::Format _format;
  vk::Extent3D _size;
  uint32_t _mipLevels;
  uint32_t _arrayLayers;

  static util::ValueRemapper<vk::Format, ColorFormat> _vkFormat2ColorFormat;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::OwnedByQueueVk<gr::ImageVk>)
RTTI_BIND(gr::ImageVk, gr::Image, gr::OwnedByQueueVk<gr::ImageVk>)