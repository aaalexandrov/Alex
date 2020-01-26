#pragma once

#include "buffer_desc.h"
#include "resource.h"

NAMESPACE_BEGIN(gr1)

class Buffer : public Resource {
	RTTR_ENABLE(Resource)
public:
  enum class Usage {
    Invalid,
    Vertex,
    Index,
    Uniform,
    Staging = 0x1000,
  };

  Buffer(Device &device) : Resource(device) {}

	virtual void Init(Usage usage, BufferDescPtr &bufferDesc) = 0;

  virtual Usage GetUsage() = 0;
  virtual size_t GetSize() = 0;

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

	inline BufferDescPtr const &GetBufferDescription() const { return _bufferDesc; }

protected:
	BufferDescPtr _bufferDesc;
};

NAMESPACE_END(gr1)

RTTI_BIND(gr1::Buffer, gr1::Resource)