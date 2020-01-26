#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"
#include <memory>
#include "glm/glm.hpp"

NAMESPACE_BEGIN(gr1)

struct BufferElement {
  rttr::type _type;
  size_t _offset;
  uint32_t _arraySize;
  uint32_t _location;

  void *GetPtr(void *buffer) const { return static_cast<uint8_t*>(buffer) + _offset; }

  template <typename Class>
  Class *GetPointer(void *buffer) const
  {
    ASSERT(_type == rttr::type::get<Class>());
    return reinterpret_cast<Class*>(GetPtr(buffer));
  }
};

struct BufferDesc;
using BufferDescPtr = std::shared_ptr<BufferDesc>;

struct BufferDesc {
  static BufferDescPtr Create() { return std::make_shared<BufferDesc>(); }

  BufferElement const *GetElement(std::string name) const
  {
    auto found = _elements.find(name);
    if (found == _elements.end())
      return nullptr;
    return &found->second;
  }

  void AddElement(std::string name, BufferElement &elem)
  {
    _size = std::max(_size, elem._offset + elem._type.get_sizeof());
    _elements.emplace(name, elem);
  }

  std::unordered_map<std::string, BufferElement> _elements;
  size_t _size = 0;
};

NAMESPACE_END(gr1)

