#pragma once

#include "types.h"

namespace alang {

struct FuncData : public Definition {
	TypeDesc *_signature = nullptr;

	FuncData(String name, TypeDesc *signature);
	FuncData(ParseNode const *node);
};

struct Func : public FuncData {
	// upvalues
	// code

	Func(String name, TypeDesc *signature);
	Func(ParseNode const *node);

	Error Analyze() override;
};

struct FuncExternal : public FuncData {
	using FuncType = void (*)(uint8_t *params);
	FuncType _externalFunc;

	FuncExternal(String name, TypeDesc *signature, FuncType externalFunc);
	FuncExternal(ParseNode const *node);

	Error Analyze() override;
};

}