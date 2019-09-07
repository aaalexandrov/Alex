#include "image_update_vk.h"
#include "image_vk.h"
#include "device_vk.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

ImageUpdateVk::ImageUpdateVk(ImageVk &updatedImage, ImageVk &sourceImage)
  : _image(util::SharedFromThis<ImageVk>(&updatedImage))
  , _signalAfterUpdate { updatedImage._device->CreateSemaphore() }
{
}

void ImageUpdateVk::RecordCommands(ImageVk &sourceImage)
{
}

NAMESPACE_END(gr)

