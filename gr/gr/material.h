#pragma once

#include "util/namespace.h"
#include "graphics_state.h"
#include "graphics_resource.h"

NAMESPACE_BEGIN(gr)

class Shader;

class Material : public GraphicsResource {
public:
  Material(std::shared_ptr<Shader> &shader) : _shader(shader) {}

  virtual GraphicsState *GetState() = 0;

  std::shared_ptr<Shader> _shader;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::Material, gr::GraphicsResource)