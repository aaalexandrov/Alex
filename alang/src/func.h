#pragma once

#include "types.h"
#include "ir.h"

namespace alang {

struct Func {
	String _name;
	TypeDesc *_signature;
	Module *_module;

	std::vector<Instruction> _code;
	std::vector<uint8_t> _constants;
	size_t _stackSize = 0;
};

}