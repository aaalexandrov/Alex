#pragma once

#include "queue_operation_vk.h"
#include "image_vk.h"

NAMESPACE_BEGIN(gr)

class ImageUpdateVk : public QueueOperationVk {
public:
  ImageUpdateVk(ImageVk &dstImage, util::BoxWithLayer const &dstRegion, uint32_t dstMinMipLevel, uint32_t dstMaxMipLevel, ImageVk &srcImage, glm::uvec4 srcPos, uint32_t srcMinMipLevel);

  void Prepare(OperationQueue *operationQueue) override;
  void Execute(OperationQueue *operationQueue) override;

  std::shared_ptr<ImageVk> _dstImage;
  std::shared_ptr<ImageVk> _srcImage;
  vk::UniqueCommandBuffer _transferCmds;
  std::unique_ptr<ImageVk::QueueTransition> _transitionToTransfer;
  util::BoxWithLayer _dstRegion;
  glm::uvec4 _srcPos;
  uint32_t _dstMinMipLevel, _dstMaxMipLevel, _srcMinMipLevel;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::ImageUpdateVk, gr::QueueOperationVk)