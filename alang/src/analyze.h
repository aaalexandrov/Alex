#pragma once

#include "module.h"
#include "parse.h"

namespace alang {

struct Analyzer {
	struct Error {
		Parser::Node *_node = nullptr;
		String _error;
	};

	std::unordered_map<String, std::unique_ptr<Module>> _modules;

	Error AnalyzeDefinitions(Parser::Node *ast);
protected:
	Error AnalyzeModuleDefinitions(Parser::Node *mod);
};

}