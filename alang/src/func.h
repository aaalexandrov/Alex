#pragma once

#include "types.h"
#include "ir.h"

namespace alang {

struct FuncData : public Definition {
	TypeDesc *_signature;

	FuncData(String name, TypeDesc *signature);
};

struct Func : public FuncData {
	std::vector<Instruction> _code;
	std::vector<uint8_t> _constants;
	size_t _stackSize = 0;

	Func(String name, TypeDesc *signature);
};

struct FuncExternal : public FuncData {
	using FuncType = void (*)(uint8_t *params);
	FuncType _externalFunc;
	uintptr_t _indirectionAddress;

	FuncExternal(String name, TypeDesc *signature, FuncType externalFunc);
};

}