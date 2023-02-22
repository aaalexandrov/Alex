#include "analyze.h"
#include "error.h"

namespace alang {

auto Analyzer::AnalyzeDefinitions(ParseNode const *ast, Module *parent) -> Error
{
	Error err;
	std::unique_ptr<Module> module = AnalyzeModuleDefinitions(ast, err);
	if (!module)
		return err;

	if (parent)
		parent->RegisterDefinition(std::move(module));
	else
		_modules[module->_name] = std::move(module);

	return Error();
}

std::unique_ptr<Module> Analyzer::AnalyzeModuleDefinitions(ParseNode const *mod, Error &err)
{
	if (mod->_label != "module") {
		err = Error(Err::ExpectedModule, mod);
		return nullptr;
	}

	auto module = std::make_unique<Module>(mod->GetToken(0)->_str, mod);

	for (int i = 1; i < mod->GetContentSize(); ++i) {
		ParseNode const *sub = mod->GetSubnode(i);
		if (sub->_label == "import") {
			err = AnalyzeImport(sub, module->_imports);
		} else if (sub->_label == "var" || sub->_label == "const") {
			auto val = AnalyzeValueDefinition(sub, err);
			if (!val)
				return nullptr;
			module->RegisterDefinition(std::move(val));
		} else if (sub->_label == "func") {
			auto func = AnalyzeFuncDefinition(sub, err);
			if (!func)
				return nullptr;
			module->RegisterDefinition(std::move(func));
		} else if (sub->_label == "module") {
			auto subMod = AnalyzeModuleDefinitions(mod, err);
			if (!subMod)
				return nullptr;
			module->RegisterDefinition(std::move(subMod));
		} else {
			err = Error(Err::UnexpectedDefinition, sub);
			return nullptr;
		}
	}

	return module;
}

std::unique_ptr<VarDef> Analyzer::AnalyzeValueDefinition(ParseNode const *val, Error &err)
{
	return nullptr;
}

std::unique_ptr<FuncData> Analyzer::AnalyzeFuncDefinition(ParseNode const *func, Error &err)
{
	if (func->_label != "func") {
		err = Error(Err::ExpectedFunc, func);
		return nullptr;
	}

	std::unique_ptr<TypeDesc> funcSignature;
	for (int i = 1; i < func->GetContentSize(); ++i) {
		ParseNode const *sub = func->GetSubnode(i);
		if (!sub || sub->_label != ":")
			break;


	}

	std::unique_ptr<FuncData> funcData;

	return funcData;
}

auto Analyzer::AnalyzeImport(ParseNode const *import, std::vector<Import> &imports) -> Error
{
	if (import->_label != "import")
		return Error(Err::ExpectedImport, import);

	for (int i = 0; i < import->GetContentSize(); ++i) {
		Import imp;
		Error err = ReadQualifiedName(imp._qualifiedName, import->_content[i]);
		if (!err._error.empty())
			return err;
		imports.push_back(imp);
	}

	return Error();
}

auto Analyzer::ReadQualifiedName(String &qualifiedName, ParseNode::Content const &name) -> Error
{
	ASSERT(qualifiedName.empty());
	if (auto *tok = name.GetToken()) {
		qualifiedName = tok->_str;
		return Error();
	}

	auto *sub = name.GetNode();
	if (sub->_label != ".")
		return Error(Err::ExpectedQualifiedName, sub);
	for (int j = 0; j < sub->GetContentSize(); ++j) {
		if (!qualifiedName.empty())
			qualifiedName += ".";
		qualifiedName += sub->GetToken(j)->_str;
	}

	return Error();
}

}