#pragma once

#include "../image.h"
#include "vk.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;
struct QueueVk;

class ImageVk : public Image {
	RTTR_ENABLE(Image)
public:
	ImageVk(Device &device) : Image(device) {}

	void Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels) override;
	void Init(Usage usage, vk::Image image, vk::Format format, glm::uvec4 size, uint32_t mipLevels);

	inline bool IsValid() override { return static_cast<bool>(_image); }

  void *Map() override;
  void Unmap() override;

	inline vk::Format GetVkFormat() const { return ColorFormat2Vk(_format); }

public:
	void CreateView();

  vk::ImageLayout GetEffectiveImageLayout(QueueVk *queue) const;

  static ColorFormat Vk2ColorFormat(vk::Format vkFmt) { return _vkFormat2ColorFormat.ToDst(vkFmt); }
  static vk::Format ColorFormat2Vk(ColorFormat clrFmt) { return _vkFormat2ColorFormat.ToSrc(clrFmt); }

  vk::ImageViewType GetImageViewType();
  static vk::ImageType GetImageType(glm::uvec3 const &size);
  static vk::ImageUsageFlags GetImageUsage(Usage usage);
  inline vk::ImageAspectFlags GetImageAspect() const { return GetImageAspect(GetVkFormat()); }
  static vk::ImageAspectFlags GetImageAspect(vk::Format format);
  inline vk::AccessFlags GetImageAccess() const { return GetImageAccess(_usage); }
  static vk::AccessFlags GetImageAccess(Usage usage);
  inline vk::PipelineStageFlags GetImagePipelineStages() const { return GetImagePipelineStages(_usage); }
  static vk::PipelineStageFlags GetImagePipelineStages(Usage usage);

  vk::Image _image;
  vk::UniqueImage _ownImage;
  vk::UniqueImageView _view;
  UniqueVmaAllocation _memory;
  vk::ImageLayout _layout;

  static util::ValueRemapper<vk::Format, ColorFormat> _vkFormat2ColorFormat;
};

NAMESPACE_END(gr1)

