#pragma once

#include "util/namespace.h"
#include "util/enumutl.h"
#include "buffer_desc.h"
#include "resource.h"
#include <string>
#include <memory>

NAMESPACE_BEGIN(gr1)

enum class ShaderKind {
  None = 0,
  Vertex = 1,
  Fragment = 2,

  Count = Fragment
};

DEFINE_ENUM_BIT_OPERATORS(ShaderKind)

class Shader : public Resource {
	RTTR_ENABLE(Resource)
public:
  struct UniformBufferInfo {
    std::string _name;
    ShaderKind _kind;
    uint32_t _binding;
    BufferDescPtr _uniformDesc = BufferDesc::Create();
  };

	Shader(Device &device) : Resource(device) {}

  virtual void Init(std::string const &name);

  std::string _name;
  BufferDescPtr _vertexDesc = BufferDesc::Create();
  std::vector<UniformBufferInfo> _uniformBuffers;
};

NAMESPACE_END(gr1)
