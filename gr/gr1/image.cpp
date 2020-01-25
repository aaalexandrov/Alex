#include "image.h"

#include "graphics_exception.h"

NAMESPACE_BEGIN(gr1)

void Image::Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels)
{
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

glm::uvec4 ImageData::GetPackedPitch(glm::uvec4 imgSize, uint32_t elemSize)
{
  imgSize = Image::GetEffectiveSize(imgSize);
  glm::uvec4 pitch;
  pitch.x = elemSize;
  pitch.y = pitch.x * imgSize.x;
  pitch.z = pitch.y * imgSize.y;
  pitch.w = pitch.z * imgSize.z;
  return pitch;
}

void ImageData::Copy(ImageData const &src, glm::uvec4 srcPos, ImageData const &dst, glm::uvec4 dstPos, glm::uvec4 size)
{
  ASSERT(src._size.x == dst._size.x && src._size.x);
  ASSERT(size.x);
  size = Image::GetEffectiveSize(size);
  glm::uvec4 srcSize = src.GetEffectiveSize();
  glm::uvec4 dstSize = dst.GetEffectiveSize();
  ASSERT(util::VecLessEq(srcPos + size, srcSize));
  ASSERT(util::VecLessEq(dstPos + size, dstSize));

  glm::uvec4 ind;
  uint32_t xSize = size.x * src._pitch.x;
  for (ind.w = 0; ind.w < size.w; ++ind.w) {
    for (ind.z = 0; ind.z < size.z; ++ind.z) {
      for (ind.y = 0; ind.y < size.y; ++ind.y) {
        ind.x = srcPos.x;
        uint32_t srcOffset = util::VecDot(ind, src._pitch);
        ind.x = dstPos.x;
        uint32_t dstOffset = util::VecDot(ind, dst._pitch);
        memcpy(dst._data + dstOffset, src._data + srcOffset, xSize);
      }
    }
  }
}


NAMESPACE_END(gr1)

