#include "compile.h"
#include "module.h"
#include "analyze.h"
#include "error.h"
#include <filesystem>

namespace alang {

String Compiler::GetFilePathForModule(std::vector<String> const &qualifiedName)
{
	auto checkPath = [&](int numNames) -> String {
		ASSERT(numNames > 0);
		for (auto &folder : _sourceFolders) {
			std::filesystem::path path(folder);
			String name = qualifiedName[0];
			for (int i = 1; i < numNames; ++i) {
				name += ".";
				name += qualifiedName[i];
			}
			name += GetAlangExtension();
			path.append(name);
			if (std::filesystem::exists(path))
				return path.string();
		}
		return String();
	};

	String modPath;
	for (int numNames = (int)qualifiedName.size(); numNames > 0; --numNames) {
		String path = checkPath(numNames);
		if (!path.empty())
			return path;
	}
	return String();
}

Error Compiler::ParseFile(String filePath, std::unique_ptr<ParseNode> &parsed)
{
	Tokenizer tokens(filePath, _alangRules.GetKeyStrings());
	auto parser = std::make_unique<Parser>(_alangRules._rules);
	parsed = parser->Parse(tokens);
	if (tokens.GetError()) {
		parsed = nullptr;
		return tokens.GetError();
	}

	parser->Dump(parsed.get());

	return Error();
}

Error Compiler::ScanModule(ParseNode const *mod, Module *parent)
{
	if (mod->_label != "module")
		return Error(Err::ExpectedModule, mod->_filePos);

	Analyzer analyzer(this);

	std::vector<String> modName;
	if (parent)
		parent->GetQualifiedName(modName);
	modName.push_back(mod->GetToken(0)->_str);
	Module *module;
	Error err = GetOrCreateModule(modName, module);
	if (err)
		return err;
	ASSERT(module->_parentDefinition == parent);
	module->_node = mod;

	err = analyzer.ScanModule(module);

	return err;
}

Error Compiler::CompileFile(String filePath)
{
	std::filesystem::path path(filePath);
	path.remove_filename();
	_sourceFolders.push_back(path.string());

	std::unique_ptr<ParseNode> mod;
	Error err = ParseFile(filePath, mod);
	if (err)
		return err;

	err = ScanModule(mod.get(), nullptr);
	if (err)
		return err;

	for (auto &[n, m] : _modules) {
		err = m->Analyze();
		if (err)
			return err;
	}

	_sourceFolders.pop_back();

	return Error();
}

Error Compiler::GetOrCreateModule(std::vector<String> const &qualifiedName, Module *&mod)
{
	mod = nullptr;
	auto &top = _modules[qualifiedName[0]];
	if (!top)
		top = std::make_unique<Module>(qualifiedName[0]);
	Module *parent = top.get();
	for (int i = 1; i < qualifiedName.size(); ++i) {
		Definition *def = parent->GetDefinition(qualifiedName[i]);
		Module *defMod;
		if (!def) {
			defMod = new Module(qualifiedName[i]);
			parent->RegisterDefinition(std::unique_ptr<Definition>(defMod));
			parent = defMod;
			continue;
		}

		parent = rtti::Cast<Module>(def);
		if (def && !parent) 
			return Error(Err::DefinitionNotModule, def->_node->_filePos);
	}

	mod = parent;
	return Error();
}

}