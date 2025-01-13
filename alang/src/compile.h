#pragma once

#include "common.h"
#include "parse.h"
#include "defs.h"
#include <unordered_map>

namespace alang {

struct Compiler {
	Compiler();

	Error AddSourceFolder(String folder);
	Error RemoveSourceFolder(String folder);

	Error ParseFile(String filePath, ParseNode const *&parsed);

	Error CompileFile(String filePath, ModuleDef *&module);

	Error ScanModule(ModuleDef *parentDef, ParseNode const *node, ModuleDef *&module);
	Error ProcessImports(ModuleDef *module);

	String GetAlangExtension() const { return ".al"; }
	String GetFilePathForModule(ParseNode::Content const *symbol);

	ParseRulesHolder const &_alangRules = AlangRules();
	std::unordered_map<String, std::unique_ptr<ParseNode>> _parsedFiles;
	std::vector<String> _sourceFolders;
	std::unique_ptr<ModuleDef> _rootModule;
};

String GetNodeName(ParseNode::Content const *content);
bool ForQualifiedNameIdentifiers(ParseNode::Content const *content, std::function<bool(String const &ident)> enumFn);

}