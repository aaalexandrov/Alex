#include "analyze.h"
#include "error.h"
#include "compile.h"

namespace alang {

Analyzer::Analyzer(Compiler *compiler)
	:_compiler(compiler)
{
}

Error Analyzer::ScanModule(Module *mod)
{
	ASSERT(mod->_node->_label == "module");

	Error err;
	for (int i = 1; i < mod->_node->GetContentSize(); ++i) {
		Token const *tok = mod->_node->GetToken(i);
		if (tok)
			return Error(Err::UnexpectedDefinition, tok->_filePos);

		ParseNode const *sub = mod->_node->GetSubnode(i);
		if (sub->_label == "import")
			err = ScanImports(sub, mod->_imports);
		else if (sub->_label == "module") {
			err = _compiler->ScanModule(sub, mod);
		} else if (sub->_label == "func")
			err = ScanFunction(sub, mod);
		else if (sub->_label == "const" || sub->_label == "var")
			err = ScanValue(sub, mod);
		else
			err = Error(Err::UnexpectedDefinition, sub->_filePos);

		if (err)
			return err;
	}

	return Error();
}

Error Analyzer::ScanImports(ParseNode const *import, std::vector<Module*> &imports)
{
	ASSERT(import->_label == "import");
	for (int i = 0; i < import->GetContentSize(); ++i) {
		std::vector<String> qualifiedName;
		Error err = ReadQualifiedName(qualifiedName, import->_content[i]);
		if (err)
			return err;
		Module *mod;
		err = _compiler->GetOrCreateModule(qualifiedName, mod);
		if (err)
			return err;
		imports.push_back(mod);
	}

	return Error();
}

Error Analyzer::ScanFunction(ParseNode const *func, Module *parent)
{
	ASSERT(func->_label == "func");


	return Error();
}

Error Analyzer::ScanValue(ParseNode const *val, Module *parent)
{
	ASSERT(val->_label == "const" || val->_label == "var");

	return Error();
}

Error Analyzer::ReadQualifiedName(std::vector<String> &qualifiedName, ParseNode::Content const &name)
{
	ASSERT(qualifiedName.empty());
	if (auto *tok = name.GetToken()) {
		qualifiedName.push_back(tok->_str);
		return Error();
	}

	auto *sub = name.GetNode();
	if (sub->_label != ".")
		return Error(Err::ExpectedQualifiedName, sub->_filePos);
	for (int j = 0; j < sub->GetContentSize(); ++j) {
		qualifiedName.push_back(sub->GetToken(j)->_str);
	}

	return Error();
}

}