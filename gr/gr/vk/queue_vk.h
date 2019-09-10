#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class ImageVk;

struct QueueVk {
  enum class Role {
    Graphics,
    Compute,
    Transfer,
    SparseOp,
    Present,
    Invalid,
    Count = Invalid,
  };


  void Init(DeviceVk &device, int32_t family, int32_t queueIndex, Role role);

  vk::UniqueCommandBuffer AllocateCmdBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);

  static void CmdSetImageLayout(vk::CommandBuffer cmds, ImageVk *image, vk::ImageLayout layout, vk::AccessFlags priorAccess, vk::AccessFlags followingAccess);
  static void CmdCopyImage(vk::CommandBuffer cmds, ImageVk *src, ImageVk *dst);

  vk::PipelineStageFlags GetPipelineStageFlags() const { return GetPipelineStageFlags(_role); }
  static vk::PipelineStageFlags GetPipelineStageFlags(Role role);
  vk::AccessFlags GetAccessFlags() const { return GetAccessFlags(_role); }
  static vk::AccessFlags GetAccessFlags(Role role);

  DeviceVk* _device;
  int32_t _family;
  Role _role;
  vk::Queue _queue;
  vk::UniqueCommandPool _cmdPool;
};

NAMESPACE_END(gr)