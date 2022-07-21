#pragma once

#include "types.h"
#include "ir.h"

namespace alang {

struct FuncData {
	String _name;
	TypeDesc *_signature;
	Module *_module;
};

struct Func {
	FuncData _func;

	std::vector<Instruction> _code;
	std::vector<uint8_t> _constants;
	size_t _stackSize = 0;
};

struct NativeFunc {
	FuncData _func;

	using FuncType = void (*)(uint8_t *params);
	FuncType _nativeFunc;
};

}