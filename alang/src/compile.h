#pragma once

#include "common.h"
#include "parse.h"
#include "module.h"
#include <unordered_map>

namespace alang {

struct Ast2Ir;
struct Compiler {
	String GetFilePathForModule(std::vector<String> const &qualifiedName);
	std::vector<String> GetQualifiedNameForFilePath(String filePath);

	Error ParseFile(String filePath, std::unique_ptr<ParseNode> &parsed);

	Error CompileFile(String filePath);

	Error GetOrCreateModule(std::vector<String> const &qualifiedName, Module *&mod);
	Definition *GetRegisteredDefinition(std::vector<String> const &qualifiedName);
	template<typename T>
	T *GetRegisteredDefinition(std::vector<String> const &qualifiedName) 
	{
		Definition *def = GetRegisteredDefinition(qualifiedName);
		return rtti::Cast<T>(def);
	}

	String GetAlangExtension() const { return ".al"; }

	ParseRulesHolder const &_alangRules = AlangRules();
	std::unordered_map<String, std::unique_ptr<Module>> _modules;
	std::vector<String> _sourceFolders;
};

}