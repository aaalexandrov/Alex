#include "buffer.h"

NAMESPACE_BEGIN(gr1)

void Buffer::Init(Usage usage, std::shared_ptr<util::LayoutElement> const &layout)
{
	_usage = usage;
	_layout = layout;
}

NAMESPACE_END(gr1)

