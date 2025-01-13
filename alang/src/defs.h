#pragma once

#include "common.h"
#include "parse.h"

namespace alang {

struct Compiler;
struct Def : rtti::Any {
	enum State {
		Created,
		Scanned,
		Resolved,
		Compiled,
	};

	String _name;
	Def *_parentDef = nullptr;
	ParseNode const *_node = nullptr;
	State _state = Created;

	Def(String name = String(), Def *parentDef = nullptr) : _name(name), _parentDef(parentDef) {}
	virtual ~Def() {}
	virtual Error Scan(Compiler *compiler, ParseNode const *node);
	Error Resolve(Compiler *compiler);

	virtual Error ScanImpl(Compiler *compiler) = 0;
	virtual Error ResolveImpl(Compiler *compiler) = 0;

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
	TypeDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<TypeDef, Def>(); }
};

struct FuncDef : Def {
	TypeDef const *_funcDef = nullptr;

	FuncDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<FuncDef, Def>(); }
};

struct ValueDef : Def {
	ValueDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<ValueDef, Def>(); }
};

struct ModuleDef : Def {
	std::unordered_map<String, std::unique_ptr<Def>> _definitions;
	std::unordered_multimap<String, Def *> _importedDefs;
	std::vector<ParseNode::Content const *> _importSymbolNodes;

	ModuleDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;

	Error FindDefForSymbol(ParseNode::Content const *symbol, Def *&foundDef);

	Error RegisterDef(std::unique_ptr<Def> &&def);
	Error RegisterImportedDef(Def *def, String name = "");

	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<ModuleDef, Def>(); }
};

}