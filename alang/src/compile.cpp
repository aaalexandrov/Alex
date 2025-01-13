#include "compile.h"
#include "error.h"
#include "core.h"
#include <filesystem>

namespace alang {

Compiler::Compiler()
	: _rootModule(std::make_unique<ModuleDef>("#root"))
{
	_rootModule->RegisterDef(CreateCore());
}

Error Compiler::AddSourceFolder(String folder)
{
	ASSERT(std::find(_sourceFolders.begin(), _sourceFolders.end(), folder) == _sourceFolders.end());
	_sourceFolders.push_back(folder);
	return Error();
}

Error Compiler::RemoveSourceFolder(String folder)
{
	auto it = std::find(_sourceFolders.begin(), _sourceFolders.end(), folder);
	ASSERT(it != _sourceFolders.end());
	_sourceFolders.erase(it);
	return Error();
}

Error Compiler::ParseFile(String filePath, ParseNode const *&parsed)
{
	Tokenizer tokens(filePath, _alangRules.GetKeyStrings());
	auto parser = std::make_unique<Parser>(_alangRules._rules);
	auto ownParsed = parser->Parse(tokens);
	if (tokens.GetError()) {
		parsed = nullptr;
		return tokens.GetError();
	}

	parsed = ownParsed.get();
	String canonicalPath = std::filesystem::weakly_canonical(filePath).string();
	_parsedFiles.insert(std::pair(canonicalPath, std::move(ownParsed)));

	parser->Dump(parsed);

	return Error();
}

Error Compiler::CompileFile(String filePath, ModuleDef *&module)
{
	module = nullptr;
	ParseNode const *parsed;
	Error err = ParseFile(filePath, parsed);
	if (err)
		return err;

	err = ScanModule(_rootModule.get(), parsed, module);

	return err;
}

String Compiler::GetFilePathForModule(ParseNode::Content const *symbol)
{
	std::vector<String> qualifiedName;
	ForQualifiedNameIdentifiers(symbol, [&](String const &ident) {
		qualifiedName.push_back(ident);
		return true;
	});

	for (int i = (int)qualifiedName.size(); i > 0; --i) {
		String name = ConcatString(qualifiedName.begin(), qualifiedName.begin() + i, '.');
		name += GetAlangExtension();
		for (auto &folder : _sourceFolders) {
			std::filesystem::path path(folder);
			path.append(name);
			if (std::filesystem::exists(path))
				return path.string();
		}
	}
	return String();
}

Error Compiler::ScanModule(ModuleDef *parentDef, ParseNode const *node, ModuleDef *&module)
{
	ASSERT(parentDef);
	module = nullptr;
	ASSERT(node->GetContentSize() > 0);
	Error err;
	bool res = ForQualifiedNameIdentifiers(&node->_content[0], [&](String const &ident) {
		std::unique_ptr<Def> &modPtr = parentDef->_definitions[ident];
		if (!modPtr) {
			modPtr = std::make_unique<ModuleDef>(ident, parentDef);
		}
		parentDef = rtti::Cast<ModuleDef>(modPtr.get());
		if (!parentDef) {
			err = Error(Err::ExpectedModule, modPtr->_node->_filePos);;
			return false;
		}
		return true;
	});
	if (!res)
		return err;

	module = parentDef;
	ASSERT(module);

	err = module->Scan(this, node);
	if (err)
		return err;

    return Error();
}

Error Compiler::ProcessImports(ModuleDef *module)
{
	ASSERT(module->_state == Def::Scanned);
	Def *coreModule = _rootModule->_definitions["Core"].get();
	Error err = module->RegisterImportedDef(coreModule);
	if (err)
		return err;
	for (ParseNode::Content const *sym : module->_importSymbolNodes) {
		Def *foundDef = nullptr;
		err = module->FindDefForSymbol(sym, foundDef);
		ASSERT(bool(err) == !foundDef);
		if (!foundDef || foundDef->_state < Def::Scanned) {
			String modPath = GetFilePathForModule(sym);
			if (modPath.empty()) 
				return Error(Err::PathForModuleNotFound, sym->GetFilePos());
			ModuleDef *mod; 
			err = CompileFile(modPath, mod);
			ASSERT(bool(err) == !mod);
			if (err)
				return err;
			// search for the def again because we might have loaded a parent module
			err = module->FindDefForSymbol(sym, foundDef);
			if (err)
				return err;
		}
		ASSERT(foundDef);
		ModuleDef *foundMod = rtti::Cast<ModuleDef>(foundDef);
		if (foundMod) {
			for (auto &[name, def] : foundMod->_definitions) {
				err = module->RegisterImportedDef(def.get());
				if (err)
					return err;
			}
		} else {
			err = module->RegisterImportedDef(foundDef);
			if (err)
				return err;
		}
	}

	return Error();
}

String GetNodeName(ParseNode::Content const *content)
{
	if (Token const *token = content->GetToken(); token && token->GetClass() == Token::Class::Identifier)
		return token->_str;
	ASSERT(0);
	return String();
}

bool ForQualifiedNameIdentifiers(ParseNode::Content const *content, std::function<bool(String const &ident)> enumFn)
{
	if (Token const *token = content->GetToken(); token && token->GetClass() == Token::Class::Identifier) {
		if (!enumFn(token->_str))
			return false;
	} else if (ParseNode const *sub = content->GetNode(); sub && sub->_label == ".") {
		for (int i = 0; i < sub->GetContentSize(); ++i) {
			auto *ident = sub->GetToken(i);
			ASSERT(ident && ident->GetClass() == Token::Class::Identifier);
			if (!enumFn(ident->_str))
				return false;
		}
	}
	return true;
}

}