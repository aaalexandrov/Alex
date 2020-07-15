#pragma once

#include "../buffer.h"

NAMESPACE_BEGIN(gr1)

class BufferContent {
	RTTR_ENABLE()
public:

	BufferContent() = default;
	BufferContent(std::shared_ptr<Buffer> const &buffer) { SetBuffer(buffer); }

	void SetBuffer(std::shared_ptr<Buffer> const &buffer);

	template<typename DataType, typename... Indices>
	DataType *GetMemberPtr(Indices... indices) const { return _buffer->GetBufferLayout()->GetMemberPtr<DataType>(_bufferData.data(), indices...); }

public:
	std::shared_ptr<Buffer> _buffer;
	std::vector<uint8_t> _bufferData;
};

NAMESPACE_END(gr1)