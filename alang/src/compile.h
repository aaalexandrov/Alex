#pragma once

#include "common.h"
#include "parse.h"
#include <unordered_map>

namespace alang {

struct Compiler {
	String GetFilePathForModule(std::vector<String> const &qualifiedName);

	Error ParseFile(String filePath, std::unique_ptr<ParseNode> &parsed);

	Error ScanModule(ParseNode const *mod);

	Error CompileFile(String filePath);

	ParseRulesHolder const &_alangRules = AlangRules();
	std::unordered_map<String, std::unique_ptr<Module>> _modules;
};

}