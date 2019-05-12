#pragma once

#include "util/namespace.h"
#include <string>

NAMESPACE_BEGIN(gr)

class Shader {
public:
  Shader(std::string const &name) : _name(name) {}
  virtual ~Shader() {}

  std::string _name;
};

NAMESPACE_END(gr)