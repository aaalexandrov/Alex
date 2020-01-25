#include "queue_vk.h"
#include "device_vk.h"
#include "../graphics_exception.h"
#include "util/mathutl.h"

NAMESPACE_BEGIN(gr1)

void QueueVk::Init(DeviceVk &device, int32_t family, int32_t queueIndex, QueueRole role)
{
  _device = &device;
  _family = family;
  _queue = device._device->getQueue(family, queueIndex);
  _role = role;

  vk::CommandPoolCreateInfo poolInfo { vk::CommandPoolCreateFlagBits::eResetCommandBuffer, static_cast<uint32_t>(family) };
  _cmdPool = device._device->createCommandPoolUnique(poolInfo, device.AllocationCallbacks());
}

vk::UniqueCommandBuffer QueueVk::AllocateCmdBuffer(vk::CommandBufferLevel level)
{
  vk::CommandBufferAllocateInfo cmdsInfo(*_cmdPool, level, 1);
  std::vector<vk::UniqueCommandBuffer> buffers = _device->_device->allocateCommandBuffersUnique(cmdsInfo);
  return std::move(buffers[0]);
}

vk::PipelineStageFlags QueueVk::GetPipelineStageFlags(QueueRole role)
{
  vk::PipelineStageFlags flags;
  switch (role) {
    case QueueRole::Graphics:
      flags = vk::PipelineStageFlagBits::eAllGraphics;
      break;
    case QueueRole::Compute:
      flags = vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eDrawIndirect;
      break;
    case QueueRole::Transfer:
      flags = vk::PipelineStageFlagBits::eTransfer;
      break;
    case QueueRole::SparseOp:
      flags = vk::PipelineStageFlagBits::eAllCommands; // ??
      break;
    case QueueRole::Present:
      flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      break;
    default:
      throw GraphicsException("Invalid queue role", -1);
      break;
  }
  return flags;
}

vk::AccessFlags QueueVk::GetAccessFlags(QueueRole role)
{
  vk::AccessFlags flags;
  switch (role) {
    case QueueRole::Graphics:
      flags = vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eUniformRead |
        vk::AccessFlagBits::eInputAttachmentRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentRead | 
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      break;
    case QueueRole::Compute:
      flags = vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
      break;
    case QueueRole::Transfer:
      flags = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
      break;
    case QueueRole::SparseOp:
      flags = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryRead; // ??
      break;
    case QueueRole::Present:
      flags = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      break;
    default:
      throw GraphicsException("Invalid queue role", -1);
      break;
  }
  return flags;
}

NAMESPACE_END(gr1)