#pragma once

#include "../image.h"
#include "queue_vk.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;

class ImageVk : public Image {
	RTTR_ENABLE(Image)
public:
	ImageVk(Device &device) : Image(device) {}

	void Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels) override;
	void Init(Resource *owner, Usage usage, vk::Image image, vk::Format format, glm::uvec4 size, uint32_t mipLevels);

	std::shared_ptr<ResourceStateTransitionPass> CreateTransitionPass(ResourceState srcState, ResourceState dstState) override;

  void *Map() override;
  void Unmap() override;

	inline vk::Format GetVkFormat() const { return ColorFormat2Vk(_format); }

public:
	void CreateView();

  static ColorFormat Vk2ColorFormat(vk::Format vkFmt) { return s_vkFormat2ColorFormat.ToDst(vkFmt); }
  static vk::Format ColorFormat2Vk(ColorFormat clrFmt) { return s_vkFormat2ColorFormat.ToSrc(clrFmt); }

  vk::ImageViewType GetImageViewType();
  static vk::ImageType GetImageType(glm::uvec3 const &size);
  static vk::ImageUsageFlags GetImageUsage(Usage usage);
  inline vk::ImageAspectFlags GetImageAspect() const { return GetImageAspect(GetVkFormat()); }
  static vk::ImageAspectFlags GetImageAspect(vk::Format format);

	struct StateInfo {
		vk::AccessFlags _access;
		vk::ImageLayout _layout;
		vk::PipelineStageFlags _stages;
		QueueRole _queueRole = QueueRole::Invalid;

		bool IsValid() { return _stages && _queueRole != QueueRole::Invalid; }
	};

	inline StateInfo GetStateInfo(ResourceState state) { return GetStateInfo(state, _usage); }
	static StateInfo GetStateInfo(ResourceState state, Usage usage);

  vk::Image _image;
  vk::UniqueImage _ownImage;
  vk::UniqueImageView _view;
  UniqueVmaAllocation _memory;
	Resource *_owner = nullptr;

  static util::ValueRemapper<vk::Format, ColorFormat> s_vkFormat2ColorFormat;
};

NAMESPACE_END(gr1)

