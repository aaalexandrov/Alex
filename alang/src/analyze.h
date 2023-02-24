#pragma once

#include "module.h"
#include "parse.h"

namespace alang {

struct Analyzer {
	std::unordered_map<String, std::unique_ptr<Module>> _modules;

	Error AnalyzeDefinitions(ParseNode const *ast, Module *parent);
protected:
	std::unique_ptr<Module> AnalyzeModuleDefinitions(ParseNode const *mod, Error &err);

	std::unique_ptr<VarDef> AnalyzeValueDefinition(ParseNode const *val, Error &err);
	std::unique_ptr<FuncData> AnalyzeFuncDefinition(ParseNode const *func, Error &err);

	Error AnalyzeImport(ParseNode const *import, std::vector<Import> &imports);

	Error ReadQualifiedName(String &qualifiedName, ParseNode::Content const &name);
};

}