#pragma once

#include "util/namespace.h"
#include "util/rtti.h"
#include <string>

NAMESPACE_BEGIN(gr)

struct BufferElement {
  util::TypeInfo *_type;
  size_t _offset;
  uint32_t _location, _binding;

  void *GetPtr(void *buffer) const { return static_cast<uint8_t*>(buffer) + _offset; }

  template <typename Class>
  Class *GetPointer(void *buffer) const 
  {  
    ASSERT(_type == TypeInfo::Get<Class>());
    return reinterpret_cast<Class*>(GetPtr(buffer));
  }
};

struct BufferDesc {
  BufferElement const *GetElement(std::string name) const
  {
    auto found = _elements.find(name);
    if (found == _elements.end())
      return nullptr;
    return &found->second;
  }

  void AddElement(std::string name, BufferElement elem)
  {
    _elements.emplace(std::make_pair(name, elem));
  }

  std::unordered_map<std::string, BufferElement> _elements;
};

class Shader {
public:
  struct UniformBufferInfo {
    BufferDesc _uniformDesc;
    uint32_t _binding;
    std::string _name;
  };

  Shader(std::string const &name) : _name(name) {}
  virtual ~Shader() {}

  std::string _name;
  BufferDesc _vertexDesc;
  std::vector<UniformBufferInfo> _uniformBuffers;
};

NAMESPACE_END(gr)

RTTI_BIND(glm::vec2)
RTTI_BIND(glm::vec3)
RTTI_BIND(glm::vec4)

RTTI_BIND(glm::mat2)
RTTI_BIND(glm::mat3)
RTTI_BIND(glm::mat4)