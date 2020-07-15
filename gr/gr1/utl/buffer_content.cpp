#include "buffer_content.h"

NAMESPACE_BEGIN(gr1)

void BufferContent::SetBuffer(std::shared_ptr<Buffer> const &buffer)
{
	_buffer = buffer;
	_bufferData.resize(buffer->GetSize());
	std::fill(_bufferData.begin(), _bufferData.end(), 0);
}

NAMESPACE_END(gr1)

