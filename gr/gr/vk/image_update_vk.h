#pragma once

#include "queue_operation_vk.h"
#include "image_vk.h"

NAMESPACE_BEGIN(gr)

class ImageUpdateVk : public QueueOperationVk {
public:
  ImageUpdateVk(ImageVk &updatedImage, ImageVk &sourceImage);

  void Prepare(OperationQueue *operationQueue) override;
  void Execute(OperationQueue *operationQueue) override;

  std::shared_ptr<ImageVk> _image;
  std::shared_ptr<ImageVk> _stagingImage;
  vk::UniqueCommandBuffer _transferCmds;
  std::unique_ptr<ImageVk::QueueTransition> _transitionToTransfer;

};

NAMESPACE_END(gr)

RTTI_BIND(gr::ImageUpdateVk, gr::QueueOperationVk)