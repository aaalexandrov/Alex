#pragma once

#include "util/namespace.h"
#include "util/enumutl.h"
#include "buffer_desc.h"
#include "graphics_resource.h"
#include <string>
#include <memory>

NAMESPACE_BEGIN(gr)

enum class ShaderKind {
  None = 0,
  Vertex = 1,
  Fragment = 2,

  Count = Fragment
};

DEFINE_ENUM_BIT_OPERATORS(ShaderKind)

class Shader : public GraphicsResource {
public:
  struct UniformBufferInfo {
    std::string _name;
    ShaderKind _kind;
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

RTTI_BIND(gr::Shader, gr::GraphicsResource)