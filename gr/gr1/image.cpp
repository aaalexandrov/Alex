#include "image.h"

#include "graphics_exception.h"

NAMESPACE_BEGIN(gr1)

void Image::Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels)
{
	ASSERT(mipLevels > 0);
	_usage = usage;
	_format = format;
	_size = size;
	_mipLevels = mipLevels;
}

uint32_t Image::GetColorFormatSize(ColorFormat format)
{
  switch (format) {
    case ColorFormat::D24S8:
    case ColorFormat::R8G8B8A8:
      return 4;
  }
  throw GraphicsException("Image::GetColorFormatSize(): Unknown color format!", -1);
}

NAMESPACE_END(gr1)

