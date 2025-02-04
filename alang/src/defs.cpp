#include "defs.h"

#include "parse.h"
#include "compile.h"
#include "error.h"

#include <memory.h>

namespace alang {

ConstValue::ConstValue(ConstValue const &c)
{
	operator=(c);
}

ConstValue::ConstValue(ConstValue &&c)
{
	operator=(std::move(c));
}

ConstValue::~ConstValue()
{
	SetType(nullptr);
}

ConstValue &ConstValue::operator=(ConstValue const &c)
{
	SetType(c._type);
	if (!_type)
		return *this;
	memcpy(GetValue(), c.GetValue(), _type->_size);
	return *this;
}

ConstValue &ConstValue::operator=(ConstValue &&c)
{
	SetType(nullptr);
	_type = c._type;
	_value = c._value;
	c._type = nullptr;
	c._value = nullptr;
	return *this;
}

void *ConstValue::SetType(TypeDef const *type)
{
	if (_type && _type->_size <= sizeof(_value))
		delete [] (uint8_t*)_value;
	_type = type;
	if (_type) {
		void *val;
		if (_type->_size <= sizeof(_value)) {
			_value = new uint8_t[_type->_size];
			val = _value;
		} else {
			val = &_value;
		}
		memset(val, 0, _type->_size);
		return val;
	} else {
		_value = nullptr;
		return nullptr;
	}
}

void *ConstValue::GetValue()
{
	if (!_type)
		return nullptr;
	return _type->_size <= sizeof(_value) ? &_value : _value;
}

bool ConstValue::operator==(ConstValue const &rhs) const
{
	if (_type != rhs._type)
		return false;
	if (!_type)
		return true;
	return memcmp(GetValue(), rhs.GetValue(), _type->_size) == 0;
}

bool GenericInstantiation::operator==(GenericInstantiation const &rhs) const
{
	ASSERT(_genericDef);
	ASSERT(rhs._genericDef);
	if (_genericDef != rhs._genericDef)
		return false;
	return _paramValues != rhs._paramValues;
}


Error Def::Scan(Compiler *compiler, ParseNode const *node)
{
	ASSERT(_state < Scanned);
	_state = Scanned;
	_node = node;
	return ScanImpl(compiler);
}

Error Def::Resolve(Compiler *compiler)
{
	if (_state >= Resolved)
		return Error();
	ASSERT(_state == Scanned);
	_state = Resolving;
	Error err = ResolveImpl(compiler);
	if (!err)
		_state = Resolved;
	return err;
}

Def *Def::SetGenericParams(Parameters &&genericParams)
{
	ASSERT(_generic._params.empty());
	ASSERT(genericParams.size() > 0);
	_generic._params = std::move(genericParams);
	return this;
}

Error Def::GetGenericInstantiation(Compiler *compiler, GenericInstantiation const &instantiation, ParseNode const *instNode, Def *&def)
{
	ASSERT(instantiation._genericDef == this);
	ASSERT(instNode->_label == "{");
	def = nullptr;
	if (instantiation._paramValues.size() != _generic._params.size()) {
		return Error(Err::MismatchingGenericArguments, instNode->_filePos);
	}
	for (int i = 0; i < instantiation._paramValues.size(); ++i) {
		if (instantiation._paramValues[i]._type != _generic._params[i]._type) {
			return Error(Err::MismatchingGenericArguments, instNode->GetContentFilePos(i + 1));
		}
	}
	for (Def *inst : _generic._instantiations) {
		if (inst->_instantiation._paramValues == instantiation._paramValues) {
			def = inst;
			return Error();
		}
	}
	def = rtti::Cast<Def>(def->GetTypeInfo()->_constructor->Invoke<Any*>());
	def->_name = _name + "{";
	for (auto &paramVal : instantiation._paramValues) {
		if (def->_name.back() != '{')
			def->_name += ", ";
		def->_name += compiler->ConstToString(paramVal);
	}
	def->_name += "}";
	def->_parentDef = _parentDef;
	def->_instantiation = instantiation;
	def->_node = instNode;
	def->_state = Resolved; // ???
	return Error();
}

String Def::GetQualifiedName() const
{
	String name = _name;
	for (Def *parent = _parentDef; parent && parent->_name != "#root"; parent = parent->_parentDef) {
		name = parent->_name + "." + name;
	}
	return name;
}

ModuleDef *Def::GetParentModule() const
{
	Def *def = _parentDef;
	while (true) {
		ModuleDef *mod = rtti::Cast<ModuleDef>(def);
		if (mod)
			return mod;
		def = def->_parentDef;
	}
	return nullptr;
}

Error TypeDef::GetConstValue(Compiler *compiler, ConstValue &constVal)
{
	ModuleDef *core = rtti::Cast<ModuleDef>(compiler->_rootModule->_definitions["Core"].get());
	TypeDef **defVal = (TypeDef **)constVal.SetType(rtti::Cast<TypeDef>(core->_definitions["TypeDef"].get()));
	*defVal = this;
    return Error();
}

Error TypeDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error TypeDef::ResolveImpl(Compiler *compiler)
{
	return Error();
}

Error FuncDef::GetConstValue(Compiler *compiler, ConstValue &constVal)
{
	return Error(Err::Unimplemented, _node->_filePos);
}

Error FuncDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error FuncDef::ResolveImpl(Compiler *compiler)
{
	return Error();
}

bool ValueDef::IsConst() const
{
	return _node->_label == "const";
}

Error ValueDef::GetConstValue(Compiler *compiler, ConstValue &constVal)
{
	ASSERT(_valueType);
	if (!IsConst())
		return Error(Err::NotConst, _node->_filePos);
	constVal = _initValue;
	return Error();
}

Error ValueDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error ValueDef::ResolveImpl(Compiler *compiler)
{
	ASSERT(!_valueType);
	ModuleDef *mod = GetParentModule();
	ParseNode const *ofType = _node->GetSubnode(1);
	ASSERT(ofType->_label == ":");
	Def *def;
	Error err = mod->FindDefForSymbol(compiler, ofType->GetContent(0), def);
	if (err)
		return err;
	_valueType = rtti::Cast<TypeDef>(def);
	if (!_valueType)
		return Error(Err::ExpectedType, ofType->GetContentFilePos(0));
	ParseNode const *init = _node->GetSubnode(2);
	if (!init) {
		ASSERT(!IsConst());
		return Error();
	}
	ASSERT(init->_label == "=");
	_initValue.SetType(_valueType);
	err = compiler->ReadLiteralConst(init->GetContent(0), _initValue);
	if (err)
		return err;
	return Error();
}

Error ModuleDef::GetConstValue(Compiler *compiler, ConstValue &constVal)
{
	return Error(Err::Unimplemented, _node->_filePos);
}

Error ModuleDef::ScanImpl(Compiler *compiler)
{
	for (int i = 1; i < _node->GetContentSize(); ++i) {
		ParseNode const *sub = _node->GetSubnode(i);
		ASSERT(sub);
		std::unique_ptr<Def> newDef;
		Error err;
		if (sub->_label == "const" || sub->_label == "var") {
			newDef = std::make_unique<ValueDef>(GetNodeName(sub->GetContent(0)), this);
			err = newDef->Scan(compiler, sub);
		} else if (sub->_label == "func") {
			newDef = std::make_unique<FuncDef>(GetNodeName(sub->GetContent(0)), this);
			err = newDef->Scan(compiler, sub);
		} else if (sub->_label == "module") {
			ModuleDef *subMod;
			err = compiler->ScanModule(this, sub, subMod);
		} else if (sub->_label == "import") {
			for (int i = 0; i < sub->GetContentSize(); ++i) {
				_importSymbolNodes.push_back(sub->GetContent(i));
			}
		} else {
			ASSERT(0);
		}
		if (err)
			return err;
		if (newDef) {
			err = RegisterDef(std::move(newDef));
			if (err)
				return err;
		}
	}

	return Error();
}

Error ModuleDef::ResolveImpl(Compiler *compiler)
{
	for (auto &[name, def] : _definitions) {
		Error err = def->Resolve(compiler);
		if (err)
			return err;
	}
	return Error();
}

Error ModuleDef::FindDefForSymbol(Compiler *compiler, ParseNode::Content const *symbol, Def *&foundDef)
{
	bool hasMultipleMatches = false;
	auto getDef = [&](ModuleDef *module, String const &name) -> Def* {
		if (!module)
			return nullptr;
		if (auto it = module->_definitions.find(name); it != module->_definitions.end()) {
			return it->second.get();
		}
		auto it = module->_importedDefs.find(name);
		if (it != module->_importedDefs.end()) {
			auto itNext = std::next(it);
			bool multipleMatches = itNext != module->_importedDefs.end() && itNext->first == name;
			hasMultipleMatches |= multipleMatches;
			if (!multipleMatches)
				return it->second;
		}
		return nullptr;
	};
	Error err;
	auto findSymbol = [&](ModuleDef *module, ParseNode::Content const *symbol) -> Def* {
		if (ParseNode const *symNode = symbol->GetNode()) {
			if (symNode->_label == "{") {
				GenericInstantiation instantiation;
				err = ReadGenericInstantiation(compiler, symNode, instantiation);
				if (err)
					return nullptr;
				Def *instDef;
				err = instantiation._genericDef->GetGenericInstantiation(compiler, instantiation, symNode, instDef);
				if (err)
					return nullptr;
				ASSERT(instDef);
				return instDef;
			} else {
				ASSERT(symNode->_label == ".");
				Def *curDef = module;
				for (int i = 0; curDef && i < symNode->GetContentSize(); ++i) {
					Token const *subToken = symNode->GetToken(i);
					ASSERT(subToken && subToken->GetClass() == Token::Class::Identifier);
					curDef = getDef(rtti::Cast<ModuleDef>(curDef), subToken->_str);
				}
				return curDef;
			}
		} else {
			Token const *symToken = symbol->GetToken();
			ASSERT(symToken->GetClass() == Token::Class::Identifier);
			return getDef(module, symToken->_str);
		}
	};

	for (ModuleDef *mod = this; mod; mod = rtti::Cast<ModuleDef>(mod->_parentDef)) {
		foundDef = findSymbol(mod, symbol);
		if (foundDef)
			return Error();
		if (err)
			return err;
	}

	return Error(hasMultipleMatches ? Err::AmbiguousSymbolNotFound : Err::UndefinedSymbol, symbol->GetFilePos());
}

Error ModuleDef::ReadGenericInstantiation(Compiler *compiler, ParseNode const *node, GenericInstantiation &instantiation)
{
	ASSERT(node->_label == "{");
	ASSERT(!instantiation._genericDef);
	ASSERT(instantiation._paramValues.empty());
	Def *def;
	Error err = FindDefForSymbol(compiler, node->GetContent(0), def);
	if (err)
		return err;
	instantiation._genericDef = rtti::Cast<TypeDef>(def);
	if (!instantiation._genericDef || instantiation._genericDef->_generic._params.empty())
		return Error(Err::ExpectedGenericDefinition, node->GetContentFilePos(0));
	if (node->GetContentSize() - 1 != instantiation._genericDef->_generic._params.size())
		return Error(Err::NumberOfParametersMismatch, node->GetContentFilePos(std::min(1, node->GetContentSize())));
	for (int i = 1; i < node->GetContentSize(); ++i) {
		ParseNode::Content const *contParam = node->GetContent(i);
		if (Token const *tokParam = contParam->GetToken()) {
			// literal
		} else {
			Def *def;
			err = FindDefForSymbol(compiler, contParam, def);
			if (err)
				return err;
			err = def->GetConstValue(compiler, instantiation._paramValues.emplace_back());
			if (err)
				return err;
		}
	}

	return Error();
}

Error ModuleDef::Resolve(Compiler *compiler, ParseNode::Content const *symbol, Def *&resolvedDef)
{
	resolvedDef = nullptr;
	Def *def;
	Error err = FindDefForSymbol(compiler, symbol, def);
	if (err) 
		return err;
	err = def->Resolve(compiler);
	if (err)
		return err;
	resolvedDef = def;
	return Error();
}

Error ModuleDef::GetTypeDef(Compiler *compiler, ParseNode::Content const *symbol, ParseNode const *paramsNode, TypeDef *&typeDef)
{
	ASSERT(!paramsNode || paramsNode->_label == "{");
	TypeDef *baseType;
	Error err = Resolve(compiler, symbol, baseType);
	if (err)
		return err;
	if (paramsNode) {
		// construct the instantiation name, search for it in the generic base module's definitions, and create an instantiation there if it's missing
		if (!baseType->IsGenericDef())
			return Error(Err::UnexpectedTypeParameters, paramsNode->_filePos);
		String name = baseType->_name + "{";
		for (int i = 0; i < paramsNode->GetContentSize(); ++i) {
			//err = GetTypeDef(compiler, )
		}


	} else {
		if (baseType->IsGenericDef())
			return Error(Err::ExpectedTypeParameters, symbol->GetFilePos());
		typeDef = baseType;
	}
	return Error();
}

Error ModuleDef::RegisterDef(std::unique_ptr<Def> &&def)
{
	ASSERT(def);
	if (!def->_parentDef)
		def->_parentDef = this;
	auto &registered = _definitions[def->_name];
	if (registered)
		return Error(Err::DuplicateDefinition, def->_node->_filePos);
	registered = std::move(def);
    return Error();
}

Error ModuleDef::RegisterImportedDef(Def *def, String name)
{
	if (name.empty())
		name = def->_name;
	for (auto [it, itEnd] = _importedDefs.equal_range(name); it != itEnd; ++it) {
		if (it->second == def)
			return Error();
	}
	_importedDefs.emplace(name, def);
	return Error();
}

}