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
	_state = Resolved;
	return ResolveImpl(compiler);
}

Error Def::ResolveImpl(Compiler *compiler)
{
	return Error();
}

Error TypeDef::ScanImpl(Compiler *compiler)
{
	return Error();
}

Error TypeDef::ResolveImpl(Compiler *compiler)
{
	Error err = Def::ResolveImpl(compiler);
	if (err)
		return err;
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