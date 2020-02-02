#pragma once

#include "resource.h"
#include "util/rect.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

enum class ColorFormat {
  Invalid,
  R8G8B8A8,
	B8G8R8A8,

  D24S8,
};

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

  glm::uvec4 GetEffectiveSize() const { return GetEffectiveSize(_size); }
  static glm::uvec4 GetEffectiveSize(glm::uvec4 s) { return util::VecMax(s, glm::uvec4(1, 1, 1, 1)); }

  Usage _usage = Usage::Invalid;
  ColorFormat _format = ColorFormat::Invalid;
  glm::uvec4 _size;
  uint32_t _mipLevels;
};

struct ImageData {
  glm::uvec4 _size;
  glm::uvec4 _pitch;
  uint8_t *_data;

  ImageData(glm::uvec4 size, glm::uvec4 pitch, void *data) : _size(size), _pitch(pitch), _data(static_cast<uint8_t*>(data)) { ASSERT(_size.x && _pitch.x); }

  glm::uvec4 GetEffectiveSize() const { return Image::GetEffectiveSize(_size); }

  static glm::uvec4 GetPackedPitch(glm::uvec4 imgSize, uint32_t elemSize);
  static void Copy(ImageData const &src, glm::uvec4 srcPos, ImageData const &dst, glm::uvec4 dstPos, glm::uvec4 size);
};

DEFINE_ENUM_BIT_OPERATORS(Image::Usage);

NAMESPACE_END(gr1)

