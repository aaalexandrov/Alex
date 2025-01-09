#pragma once

#include "common.h"
#include "parse.h"
#include "defs.h"
#include <unordered_map>

namespace alang {

struct Compiler {
	Compiler();

	String GetFilePathForModule(std::vector<String> const &qualifiedName);

	Error ParseFile(String filePath, ParseNode const *&parsed);

	Error CompileFile(String filePath);

	Error ScanModule(ParseNode const *node, ModuleDef *&module);

	String GetAlangExtension() const { return ".al"; }

	ParseRulesHolder const &_alangRules = AlangRules();
	std::unordered_map<String, std::unique_ptr<Def>> _modules;
	std::unordered_map<String, std::unique_ptr<ParseNode>> _parsedFiles;
	std::vector<String> _sourceFolders;
};

}