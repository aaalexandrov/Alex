#include "analyze.h"
#include "error.h"

namespace alang {

auto Analyzer::AnalyzeDefinitions(Parser::Node const *ast, Module *parent) -> Error
{
	Error err = AnalyzeModuleDefinitions(ast, parent);
	return err;
}

auto Analyzer::AnalyzeModuleDefinitions(Parser::Node const *mod, Module *parent) -> Error
{
	if (mod->_label != "module")
		return Error(Err::ExpectedModule, mod);

	auto modPtr = std::make_unique<Module>(mod->GetToken(0)->_str);
	Module *module = modPtr.get();
	if (parent)
		parent->RegisterDefinition(std::unique_ptr<Definition>(modPtr.release()));
	else
		_modules[module->_name] = std::move(modPtr);

	for (int i = 1; i < mod->GetContentSize(); ++i) {
		Error error;
		Parser::Node const *sub = mod->GetSubnode(i);
		if (sub->_label == "import")
			error = AnalyzeImport(sub, module);
		else if (sub->_label == "var" || sub->_label == "const")
			error = AnalyzeValueDefinition(sub, module);
		else if (sub->_label == "func")
			error = AnalyzeFuncDefinition(sub, module);
		else if (sub->_label == "module")
			error = AnalyzeModuleDefinitions(mod, module);
		else
			error = Error(Err::UnexpectedDefinition, sub);

		if (!error._error.empty())
			return error;
	}

	return Error();
}

auto Analyzer::AnalyzeImport(Parser::Node const *import, Module *parent) -> Error
{
	if (import->_label != "import")
		return Error(Err::ExpectedImport, import);

	for (int i = 0; i < import->GetContentSize(); ++i) {
		Import imp;
		Error err = ReadQualifiedName(imp._qualifiedName, import->_content[i]);
		if (!err._error.empty())
			return err;
		parent->_imports.push_back(imp);
	}

	return Error();
}

auto Analyzer::AnalyzeValueDefinition(Parser::Node const *value, Module *parent) -> Error
{
	return Error();
}

auto Analyzer::AnalyzeFuncDefinition(Parser::Node const *func, Module *parent) -> Error
{
	if (func->_label != "func")
		return Error(Err::ExpectedFunc, func);



	return Error();
}

auto Analyzer::ReadQualifiedName(std::vector<String> &qualifiedName, Parser::Node::Content const &name) -> Error
{
	if (auto *tok = std::get_if<Token>(&name)) {
		qualifiedName.push_back(tok->_str);
		return Error();
	}

	auto *sub = std::get<std::unique_ptr<Parser::Node>>(name).get();
	if (sub->_label != ".")
		return Error(Err::ExpectedQualifiedName, sub);
	for (int j = 0; j < sub->GetContentSize(); ++j) {
		qualifiedName.push_back(sub->GetToken(j)->_str);
	}

	return Error();
}

}