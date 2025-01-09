#pragma once

#include "common.h"

namespace alang {

struct ParseNode;
struct Def : rtti::Any {
	String _name;
	Def *_parentDef = nullptr;
	ParseNode const *_node = nullptr;

	Def(String name = String(), Def *parentDef = nullptr) : _name(name), _parentDef(parentDef) {}
	virtual ~Def() {}
	virtual Error Init(ParseNode const *node);

	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<Def, rtti::Any>(); }
};

struct TypeDef : Def {
	struct Member {
		String _name;
		TypeDef const *_type;
	};

	size_t _size = 0, _align = 0, _arraySize = 0;
	TypeDef const *_genericDef = nullptr;
	std::vector<Member> _members;

	TypeDef(String name = String(), size_t size = 0, size_t align = 0) : Def(name), _size(size), _align(align) {}

	Error Init(ParseNode const *node) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<TypeDef, Def>(); }
};

struct FuncDef : Def {
	TypeDef const *_funcDef = nullptr;

	Error Init(ParseNode const *node) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<FuncDef, Def>(); }
};

struct ValueDef : Def {
	Error Init(ParseNode const *node) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<ValueDef, Def>(); }
};

struct ModuleDef : Def {
	ParseNode const *_parsed;
	std::unordered_map<String, std::unique_ptr<Def>> _definitions;

	ModuleDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error Init(ParseNode const *node) override;

	Error RegisterDef(std::unique_ptr<Def> &&def);

	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<ModuleDef, Def>(); }
};




}