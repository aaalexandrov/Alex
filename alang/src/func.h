#pragma once

#include "types.h"

namespace alang {

struct FuncData : public Definition {
	TypeDesc *_signature = nullptr;

	FuncData(String name, TypeDesc *signature);
	FuncData() = default;

	Error Init(ParseNode const *node) override;

	rtti::TypeInfo const *GetTypeInfo() const override;

	static std::unique_ptr<TypeDesc> GetSignatureTypeDesc(ParseNode const *node);
};

struct Func : public FuncData {
	// upvalues
	// code

	Func(String name, TypeDesc *signature);
	Func() = default;

	Error Init(ParseNode const *node) override;

	rtti::TypeInfo const *GetTypeInfo() const override;
};

struct FuncExternal : public FuncData {
	using FuncType = void (*)(uint8_t *params);
	FuncType _externalFunc;

	FuncExternal(String name, TypeDesc *signature, FuncType externalFunc);
	FuncExternal() = default;

	Error Init(ParseNode const *node) override;

	rtti::TypeInfo const *GetTypeInfo() const override;
};

}

namespace rtti {
template <> inline TypeInfo const *Get<alang::FuncData>() { return GetBases<alang::FuncData, alang::Definition>(); }
template <> inline TypeInfo const *Get<alang::Func>() { return GetBases<alang::Func, alang::FuncData>(); }
template <> inline TypeInfo const *Get<alang::FuncExternal>() { return GetBases<alang::FuncExternal, alang::FuncData>(); }
}