#pragma once

#include "module.h"
#include "parse.h"

namespace alang {

struct Compiler;
struct Analyzer {
	Analyzer(Compiler *compiler);

	Error ScanModule(Module *mod);
protected:
	Error ScanImports(ParseNode const *import, std::vector<Module*> &imports);
	Error ScanFunction(ParseNode const *func, Module *parent);
	Error ScanValue(ParseNode const *val, Module *parent);

	Error ReadQualifiedName(std::vector<String> &qualifiedName, ParseNode::Content const &name);

	Compiler *_compiler = nullptr;
};

}