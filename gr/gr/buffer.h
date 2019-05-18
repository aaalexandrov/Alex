#pragma once

#include "util/namespace.h"
#include "buffer_desc.h"

NAMESPACE_BEGIN(gr)

class Buffer {
public:
  enum class Usage {
    Invalid,
    Vertex,
    Index,
    Uniform,
    Staging = 0x1000,
  };

  Buffer(BufferDescPtr &bufferDesc) : _bufferDesc(bufferDesc) {}

  virtual Usage GetUsage() = 0;
  virtual size_t GetSize() = 0;

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

  BufferDescPtr _bufferDesc;
};

NAMESPACE_END(gr)