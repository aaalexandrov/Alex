#pragma once

#include "module.h"
#include "parse.h"

namespace alang {

struct Analyzer {
	struct Error {
		String _error;
		Parser::Node const *_location = nullptr;

		Error() = default;
		Error(String err, Parser::Node const *loc) : _error(err), _location(loc) {}
	};

	std::unordered_map<String, std::unique_ptr<Module>> _modules;

	Error AnalyzeDefinitions(Parser::Node const *ast, Module *parent);
protected:
	Error AnalyzeModuleDefinitions(Parser::Node const *mod, Module *parent);

	Error AnalyzeImport(Parser::Node const *import, Module *parent);
	Error AnalyzeValueDefinition(Parser::Node const *value, Module *parent);
	Error AnalyzeFuncDefinition(Parser::Node const *func, Module *parent);

	Error ReadQualifiedName(std::vector<String> &qualifiedName, Parser::Node::Content const &name);
};

}