#pragma once

#include "queue_operation_vk.h"
#include "vk.h"

NAMESPACE_BEGIN(gr)

class ImageVk;

class ImageUpdateVk : public QueueOperationVk {
public:
  ImageUpdateVk(ImageVk &updatedImage, ImageVk &sourceImage);

  void RecordCommands(ImageVk &sourceImage);

  std::shared_ptr<ImageVk> _image;
  std::shared_ptr<ImageVk> _stagingImage;
  vk::UniqueCommandBuffer _transferCmds;
  std::shared_ptr<vk::UniqueSemaphore> _signalAfterUpdate;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::ImageUpdateVk, gr::QueueOperationVk)