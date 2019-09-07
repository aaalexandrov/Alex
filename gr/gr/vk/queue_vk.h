#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class ImageVk;

struct QueueVk {
  void Init(DeviceVk &device, int32_t family, int32_t queueIndex);

  vk::UniqueCommandBuffer &&AllocateCmdBuffer();

  static void CmdSetImageLayout(vk::CommandBuffer cmds, ImageVk *image, vk::ImageLayout layout, vk::AccessFlags priorAccess, vk::AccessFlags followingAccess);
  static void CmdCopyImage(vk::CommandBuffer cmds, ImageVk *src, ImageVk *dst);

  DeviceVk* _device;
  int32_t _family;
  vk::Queue _queue;
  vk::UniqueCommandPool _cmdPool;
};

NAMESPACE_END(gr)