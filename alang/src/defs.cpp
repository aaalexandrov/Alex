#include "defs.h"

#include "parse.h"
#include "compile.h"
#include "error.h"

namespace alang {

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
	ASSERT(_genericParams.empty());
	ASSERT(genericParams.size() > 0);
	_genericParams = std::move(genericParams);
	return this;
}

String Def::GetQualifiedName() const
{
	String name = _name;
	for (Def *parent = _parentDef; parent && parent->_name != "#root"; parent = parent->_parentDef) {
		name = parent->_name + "." + name;
	}
	return name;
}

Error TypeDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error TypeDef::ResolveImpl(Compiler *compiler)
{
	return Error();
}

Error FuncDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error FuncDef::ResolveImpl(Compiler *compiler)
{
	return Error();
}

Error ValueDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error ValueDef::ResolveImpl(Compiler *compiler)
{

	return Error();
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

Error ModuleDef::FindDefForSymbol(ParseNode::Content const *symbol, Def *&foundDef)
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
	auto findSymbol = [&](ModuleDef *module, ParseNode::Content const *symbol) -> Def* {
		if (ParseNode const *symNode = symbol->GetNode()) {
			ASSERT(symNode->_label == ".");
			Def *curDef = module;
			for (int i = 0; curDef && i < symNode->GetContentSize(); ++i) {
				Token const *subToken = symNode->GetToken(i);
				ASSERT(subToken && subToken->GetClass() == Token::Class::Identifier);
				curDef = getDef(rtti::Cast<ModuleDef>(curDef), subToken->_str);
			}
			return curDef;
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
	}

	return Error(hasMultipleMatches ? Err::AmbiguousSymbolNotFound : Err::UndefinedSymbol, symbol->GetFilePos());
}

Error ModuleDef::Resolve(Compiler *compiler, ParseNode::Content const *symbol, Def *&resolvedDef)
{
	resolvedDef = nullptr;
	Def *def;
	Error err = FindDefForSymbol(symbol, def);
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