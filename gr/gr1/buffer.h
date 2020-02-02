#pragma once

#include "buffer_desc.h"
#include "resource.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

class Buffer : public Resource {
	RTTR_ENABLE(Resource)
public:
  enum class Usage {
    Invalid = 0,
    Vertex = 1,
    Index = 2,
    Uniform = 4,
    Staging = 8,
  };

  Buffer(Device &device) : Resource(device) {}

	virtual void Init(Usage usage, BufferDescPtr &bufferDesc);

	Usage GetUsage() const { return _usage; }
	size_t GetSize() const { return _bufferDesc->_size; }

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

	inline BufferDescPtr const &GetBufferDescription() const { return _bufferDesc; }

protected:
	Usage _usage = Usage::Invalid;
	BufferDescPtr _bufferDesc;
};

DEFINE_ENUM_BIT_OPERATORS(Buffer::Usage);

NAMESPACE_END(gr1)

