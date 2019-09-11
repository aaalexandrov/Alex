#pragma once

#include "util/namespace.h"
#include "graphics_resource.h"
#include "util/rect.h"
#include "glm/glm.hpp"

NAMESPACE_BEGIN(gr)

enum class ColorFormat {
  Invalid,
  R8G8B8A8,

  D24S8,
};

class Image : public GraphicsResource {
public:
  enum class Usage {
    Invalid,
    Texture,
    Staging,
    DepthBuffer,
    RenderTarget = 0x1000,
  };

  Image(Usage usage, ColorFormat format, glm::uvec3 size, uint32_t mipLevels, uint32_t arrayLayers) 
    : _usage(usage), _format(format), _size(size), _mipLevels(mipLevels), _arrayLayers(arrayLayers)
  {}

  ColorFormat GetColorFormat() const { return _format; }
  Usage GetUsage() const { return _usage; }
  glm::uvec3 GetSize() const { return _size; }
  uint32_t GetMipLevels() const { return _mipLevels; }
  uint32_t GetArrayLayers() const { return _arrayLayers; }

  virtual void UpdateContents(void *contents, util::BoxU const &box, uint32_t mipLevel, uint32_t arrayLayer) = 0;

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

  uint32_t GetColorFormatSize() const { return GetColorFormatSize(_format); }
  static uint32_t GetColorFormatSize(ColorFormat format);

  Usage _usage;
  ColorFormat _format;
  glm::uvec3 _size;
  uint32_t _mipLevels;
  uint32_t _arrayLayers;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::Image, gr::GraphicsResource)