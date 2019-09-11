#include "image.h"

#include "graphics_exception.h"

NAMESPACE_BEGIN(gr)

uint32_t Image::GetColorFormatSize(ColorFormat format)
{
  switch (format) {
    case ColorFormat::D24S8:
    case ColorFormat::R8G8B8A8:
      return 4;
  }
  throw GraphicsException("Image::GetColorFormatSize(): Unknown color format!", -1);
}

NAMESPACE_END(gr)

