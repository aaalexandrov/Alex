#pragma once

#include "vk.h"
#include "util/rect.h"
#include "physical_device_vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class ImageVk;

struct QueueVk {
  void Init(DeviceVk &device, int32_t family, int32_t queueIndex, QueueRole role);

  vk::UniqueCommandBuffer AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  static void CmdSetImageLayout(vk::CommandBuffer cmds, ImageVk *image, vk::ImageLayout layout, vk::AccessFlags priorAccess, vk::AccessFlags followingAccess);
  static void CmdCopyImage(vk::CommandBuffer cmds, ImageVk *dst, util::BoxWithLayer const &dstRegion, uint32_t dstMinMip, uint32_t dstMaxMip, ImageVk *src, glm::uvec4 srcPos, uint32_t srcMinMip);

  vk::PipelineStageFlags GetPipelineStageFlags() const { return GetPipelineStageFlags(_role); }
  static vk::PipelineStageFlags GetPipelineStageFlags(QueueRole role);
  vk::AccessFlags GetAccessFlags() const { return GetAccessFlags(_role); }
  static vk::AccessFlags GetAccessFlags(QueueRole role);

  DeviceVk* _device;
  int32_t _family;
  QueueRole _role;
  vk::Queue _queue;
  vk::UniqueCommandPool _cmdPool;
};

NAMESPACE_END(gr)