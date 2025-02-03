#pragma once

#include "common.h"
#include "parse.h"
#include "error.h"
#include <span>

namespace alang {

struct Compiler;
struct Def;
struct TypeDef;
struct ModuleDef;

struct ConstValue {
	TypeDef const *_type = nullptr;
	void *_value = nullptr;

	ConstValue() = default;
	ConstValue(ConstValue const &c);
	ConstValue(ConstValue &&c);
	~ConstValue();

	ConstValue &operator=(ConstValue const &c);
	ConstValue &operator=(ConstValue &&c);

	void *SetType(TypeDef const *type);
	void *GetValue();
	void const *GetValue() const { return const_cast<ConstValue *>(this)->GetValue(); }

	bool operator==(ConstValue const &rhs) const;
};

struct NamedParameter {
	String _name;
	TypeDef const *_type = nullptr;
};

using Parameters = std::vector<NamedParameter>;
using ParameterConsts = std::vector<ConstValue>;

struct GenericPrototype {
	Parameters _params;
	std::vector<Def *> _instantiations;
};

struct GenericInstantiation {
	TypeDef *_genericDef = nullptr;
	ParameterConsts _paramValues;

	bool operator==(GenericInstantiation const &rhs) const;
};

struct Def : rtti::Any {
	enum State {
		Created,
		Scanned,
		Resolving,
		Resolved,
		Compiled,
	};

	String _name;
	Def *_parentDef = nullptr;
	GenericPrototype _generic;
	GenericInstantiation _instantiation;
	ParseNode const *_node = nullptr;
	State _state = Created;

	Def(String name = String(), Def *parentDef = nullptr) : _name(name), _parentDef(parentDef) {}
	virtual ~Def() {}
	virtual Error Scan(Compiler *compiler, ParseNode const *node);
	Error Resolve(Compiler *compiler);

	bool IsGenericDef() const { return _generic._params.size() > 0; }
	Def *SetGenericParams(Parameters &&genericParams);

	Error GetGenericInstantiation(Compiler *compiler, GenericInstantiation const &instantiation, ParseNode const *instNode, Def *&def);
	virtual Error GetConstValue(Compiler *compiler, ConstValue &constVal) = 0;

	String GetQualifiedName() const;
	ModuleDef *GetParentModule() const;

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
	std::vector<Member> _members;

	TypeDef(String name = String(), size_t size = 0, size_t align = 0) : Def(name), _size(size), _align(align) {}
	TypeDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error GetConstValue(Compiler *compiler, ConstValue &constVal) override;
	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<TypeDef, Def>(); }
};

struct FuncDef : Def {
	TypeDef const *_funcDef = nullptr;

	FuncDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error GetConstValue(Compiler *compiler, ConstValue &constVal) override;
	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<FuncDef, Def>(); }
};

struct ValueDef : Def {
	TypeDef const *_valueType = nullptr;
	ConstValue _initValue;

	ValueDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	bool IsConst() const;

	Error GetConstValue(Compiler *compiler, ConstValue &constVal) override;
	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;
	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<ValueDef, Def>(); }
};

struct ModuleDef : Def {
	std::unordered_map<String, std::unique_ptr<Def>> _definitions;
	std::unordered_multimap<String, Def *> _importedDefs;
	std::vector<ParseNode::Content const *> _importSymbolNodes;

	ModuleDef(String name = String(), Def *parentDef = nullptr) : Def(name, parentDef) {}

	Error GetConstValue(Compiler *compiler, ConstValue &constVal) override;
	Error ScanImpl(Compiler *compiler) override;
	Error ResolveImpl(Compiler *compiler) override;

	Error FindDefForSymbol(Compiler *compiler, ParseNode::Content const *symbol, Def *&foundDef);
	Error ReadGenericInstantiation(Compiler *compiler, ParseNode const *node, GenericInstantiation &instantiation);

	Error Resolve(Compiler *compiler, ParseNode::Content const *symbol, Def *&resolvedDef);
	template <typename DefType>
	Error Resolve(Compiler *compiler, ParseNode::Content const *symbol, DefType *&resolvedDef);

	Error GetTypeDef(Compiler *compiler, ParseNode::Content const *symbol, ParseNode const *paramsNode, TypeDef *&typeDef);

	Error RegisterDef(std::unique_ptr<Def> &&def);
	Error RegisterImportedDef(Def *def, String name = "");

	rtti::TypeInfo const *GetTypeInfo() const override { return rtti::GetBases<ModuleDef, Def>(); }
};

template<typename DefType>
inline Error ModuleDef::Resolve(Compiler *compiler, ParseNode::Content const *symbol, DefType *&resolvedDef)
{
	resolvedDef = nullptr;
	Def *def;
	Error err = Resolve(compiler, symbol, def);
	if (err)
		return err;
	resolvedDef = rtti::Cast<DefType>(def);
	return resolvedDef ? Error() : Error(Err::UnexpectedDefinitionKind, symbol->GetFilePos());
}

}