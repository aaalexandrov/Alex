#include "buffer.h"

NAMESPACE_BEGIN(gr1)

void Buffer::Init(Usage usage, BufferDescPtr & bufferDesc)
{
	_usage = usage;
	_bufferDesc = bufferDesc;
}

NAMESPACE_END(gr1)

