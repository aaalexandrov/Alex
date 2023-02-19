#pragma once

#include "types.h"

namespace alang {

struct FuncData : public Definition {
	TypeDesc *_signature;

	FuncData(String name, TypeDesc *signature);
};

struct Func : public FuncData {
	// upvalues
	// code

	Func(String name, TypeDesc *signature);
};

struct FuncExternal : public FuncData {
	using FuncType = void (*)(uint8_t *params);
	FuncType _externalFunc;

	FuncExternal(String name, TypeDesc *signature, FuncType externalFunc);
};

}