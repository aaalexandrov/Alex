#pragma once

#include "types.h"

namespace alang {

struct FuncData : public Definition {
	TypeDesc *_signature;

	FuncData(String name, ParseNode const *node, TypeDesc *signature);
};

struct Func : public FuncData {
	// upvalues
	// code

	Func(String name, ParseNode const *node, TypeDesc *signature);
};

struct FuncExternal : public FuncData {
	using FuncType = void (*)(uint8_t *params);
	FuncType _externalFunc;

	FuncExternal(String name, ParseNode const *node, TypeDesc *signature, FuncType externalFunc);
};

}