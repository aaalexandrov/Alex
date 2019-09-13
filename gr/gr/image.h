#pragma once

#include "util/namespace.h"
#include "graphics_resource.h"
#include "util/rect.h"

NAMESPACE_BEGIN(gr)

enum class ColorFormat {
  Invalid,
  R8G8B8A8,

  D24S8,
};

struct ImageData;

class Image : public GraphicsResource {
public:
  enum class Usage {
    Invalid,
    Texture,
    Staging,
    DepthBuffer,
    RenderTarget = 0x1000,
  };

  Image(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels) 
    : _usage(usage), _format(format), _size(size), _mipLevels(mipLevels)
  {}

  ColorFormat GetColorFormat() const { return _format; }
  Usage GetUsage() const { return _usage; }
  glm::uvec4 GetSize() const { return _size; }
  uint32_t GetMipLevels() const { return _mipLevels; }
  uint32_t GetArrayLayers() const { return _size.w; }

  virtual void UpdateContents(util::BoxWithLayer const &region, uint32_t mipLevel, ImageData const &content, glm::uvec4 contentPos) = 0;

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

  uint32_t GetColorFormatSize() const { return GetColorFormatSize(_format); }
  static uint32_t GetColorFormatSize(ColorFormat format);

  glm::uvec4 GetEffectiveSize() const { return GetEffectiveSize(_size); }
  static glm::uvec4 GetEffectiveSize(glm::uvec4 s) { return glm::uvec4(s.x, std::max(s.y, 1u), std::max(s.z, 1u), std::max(s.w, 1u)); }

  Usage _usage;
  ColorFormat _format;
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

NAMESPACE_END(gr)

RTTI_BIND(gr::Image, gr::GraphicsResource)