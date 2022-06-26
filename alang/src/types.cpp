#include "types.h"

namespace alang {



TypeDesc::TypeDesc(String name, size_t size, size_t align)
	: _name{ name }
	, _size{ size }
	, _align{ align }
{
}

}