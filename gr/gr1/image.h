#pragma once

#include "resource.h"
#include "util/rect.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

struct ImageData;

class Image : public Resource {
	RTTR_ENABLE(Resource)
public:
  enum class Usage {
    Invalid = 0,
    Texture = 1,
    Staging = 2,
    DepthBuffer = 4,
    RenderTarget = 8,
  };

  Image(Device &device) : Resource(device) {}

  virtual void Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels);

  ColorFormat GetColorFormat() const { return _format; }
  Usage GetUsage() const { return _usage; }
  glm::uvec4 GetSize() const { return _size; }
  uint32_t GetMipLevels() const { return _mipLevels; }
  uint32_t GetArrayLayers() const { return _size.w; }

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

  uint32_t GetColorFormatSize() const { return GetColorFormatSize(_format); }
  static uint32_t GetColorFormatSize(ColorFormat format);

  glm::uvec4 GetEffectiveSize() const { return ImageData::GetEffectiveSize(_size); }

  Usage _usage = Usage::Invalid;
  ColorFormat _format = ColorFormat::Invalid;
  glm::uvec4 _size;
  uint32_t _mipLevels;
};

DEFINE_ENUM_BIT_OPERATORS(Image::Usage);

NAMESPACE_END(gr1)

