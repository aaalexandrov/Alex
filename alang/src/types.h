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
		TypeDesc *_type = nullptr;
		size_t _offset = 0;
	};

	String _name;
	size_t _size, _align;
	Module *_module = nullptr;
	TypeDesc *_genericDefinition = nullptr;
	std::vector<Value> _params; // generic params
	std::vector<Member> _members;

	TypeDesc(String name, size_t size, size_t align);
};

}