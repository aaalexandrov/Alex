#pragma once

#include "util/namespace.h"
#include "graphics_state.h"

NAMESPACE_BEGIN(gr)

class Shader;

class Material {
public:
  Material(std::shared_ptr<Shader> &shader) : _shader(shader) {}

  GraphicsState _state;
  std::shared_ptr<Shader> _shader;
};

NAMESPACE_END(gr)