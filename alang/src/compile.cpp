#include "compile.h"
#include "error.h"
#include "core.h"
#include <filesystem>

namespace alang {

Compiler::Compiler()
{
	auto core = CreateCore();
	auto corePtr = core.get();
	_modules.insert(std::pair(corePtr->_name, std::move(core)));
}

String Compiler::GetFilePathForModule(std::vector<String> const &qualifiedName)
{
	auto checkPath = [&](int numNames) -> String {
		ASSERT(numNames > 0);
		for (auto &folder : _sourceFolders) {
			std::filesystem::path path(folder);
			String name = ConcatString(qualifiedName, '.');
			name += GetAlangExtension();
			path.append(name);
			if (std::filesystem::exists(path))
				return path.string();
		}
		return String();
	};

	return checkPath((int)qualifiedName.size());
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
	String canonicalPath = std::filesystem::weakly_canonical(filePath);
	_parsedFiles.insert(std::pair(canonicalPath, std::move(ownParsed)));

	parser->Dump(parsed);

	return Error();
}

Error Compiler::CompileFile(String filePath)
{
	std::filesystem::path path(filePath);
	path.remove_filename();
	_sourceFolders.push_back(path.string());
	ScopeGuard popFolder([&] { _sourceFolders.pop_back(); });

	ParseNode const *parsed;
	Error err = ParseFile(filePath, parsed);
	if (err)
		return err;

	ModuleDef *module;
	err = ScanModule(parsed, module);

	return err;
}

std::vector<String> GetQualifiedName(ParseNode::Content const *content)
{
	std::vector<String> name;
	if (content) {
		if (auto *token = content->GetToken(); token && token->GetClass() == Token::Class::Identifier) {
			name.push_back(token->_str);
		} else if (auto *sub = content->GetNode(); sub && sub->_label == ".") {
			for (int i = 0; i < sub->GetContentSize(); ++i) {
				auto *ident = sub->GetToken(i);
				ASSERT(ident && ident->GetClass() == Token::Class::Identifier);
				name.push_back(ident->_str);
			}
		}
	}
	return name;
}

Error Compiler::ScanModule(ParseNode const *node, ModuleDef *&module)
{
	module = nullptr;
	ASSERT(node->GetContentSize() > 0);
	std::vector<String> modName = GetQualifiedName(&node->_content[0]);
	ModuleDef *parent = nullptr;
	for (auto &ident : modName) {
		auto &modPtr = parent ? parent->_definitions[ident] : _modules[ident];
		if (!modPtr) {
			modPtr = std::make_unique<ModuleDef>(ident, parent);
		}
		parent = rtti::Cast<ModuleDef>(modPtr.get());
		if (!parent)
			return Error(Err::ExpectedModule, modPtr->_node->_filePos);
	}
	module = parent;
	ASSERT(module);

	Error err = module->Init(node);
	if (err)
		return err;

    return Error();
}

}