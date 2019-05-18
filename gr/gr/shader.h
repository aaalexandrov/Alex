#pragma once

#include "util/namespace.h"
#include "buffer_desc.h"
#include <string>
#include <memory>

NAMESPACE_BEGIN(gr)

class Shader {
public:
  struct UniformBufferInfo {
    std::string _name;
    uint32_t _binding;
    BufferDescPtr _uniformDesc = BufferDesc::Create();
  };

  Shader(std::string const &name) : _name(name) {}
  virtual ~Shader() {}

  std::string _name;
  BufferDescPtr _vertexDesc = BufferDesc::Create();
  std::vector<UniformBufferInfo> _uniformBuffers;
};

NAMESPACE_END(gr)

