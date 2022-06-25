#pragma once

#include "common.h"

namespace alang {

struct TypeDesc;
struct Value {
	TypeDesc *_type;
	uintptr_t _value;  // holds the value if _type->_size <= sizeof(uintptr_t), or a pointer to it otherwise
};

struct Module;
struct TypeDesc {
	struct Member {
		String _name;
		TypeDesc *_type;
		size_t _offset;
	};

	String _name;
	size_t _size, _align;
	Module *_module;
	TypeDesc *_genericDefinition;
	std::vector<Value> _params; // generic params
	std::vector<Member> _members;
};

}