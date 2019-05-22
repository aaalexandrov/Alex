#pragma once

#include "util/namespace.h"
#include "graphics_resource.h"
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

  virtual ColorFormat GetColorFormat() = 0;
  virtual Usage GetUsage() = 0;
  virtual glm::u32vec3 GetSize() = 0;

  virtual void *Map() = 0;
  virtual void Unmap() = 0;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::Image, gr::GraphicsResource)