#include "ast2ir.h"
#include "definitions.h"
#include "compile.h"
#include "error.h"

namespace alang {

Ast2Ir::Ast2Ir(Compiler *compiler)
	:_compiler(compiler)
{
}

void Ast2Ir::AddNodeDef(ParseNode const *node, Definition *def)
{
	ASSERT(_nodeDefs.find(node) == _nodeDefs.end());
	_nodeDefs.insert({ node, def });
}

std::vector<String> Ast2Ir::GetCurrentDefQualifiedName()
{
	std::vector<String> result;
	for (auto rit = _currentDef.rbegin(); rit != _currentDef.rend(); ++rit) {
		result.push_back((*rit)->_name);
	}
	return result;
}


Error Ast2Ir::LoadModuleDefinitions(String filepath, Module *&mod)
{
	std::unique_ptr<ParseNode> modNode;
	Error err = _compiler->ParseFile(filepath, modNode);
	if (err)
		return err;

	// when loading a module from a file, we empty the definition stack, then restore it
	std::vector<Definition *> tempDefs;
	std::swap(_currentDef, tempDefs);

	err = AnnotateModule(std::move(modNode), filepath, mod);

	std::swap(_currentDef, tempDefs);

	if (err)
		return err;

	return Error();
}

bool Ast2Ir::ModuleDefinitionsLoaded(Module *mod)
{
	return mod->_node;
}

Error Ast2Ir::AnnotateModule(ParseNode const *node, String filepath, Module *&mod)
{
	return AnnotateModuleImpl(node, false, filepath, mod);
}

Error Ast2Ir::AnnotateModule(std::unique_ptr<ParseNode const> &&node, String filepath, Module *&mod)
{
	return AnnotateModuleImpl(node.release(), true, filepath, mod);
}

Error Ast2Ir::AnnotateModuleImpl(ParseNode const *node, bool ownNode, String filepath, Module *&mod)
{
	ASSERT(mod == nullptr);
	ASSERT(node->_label == "module");
	std::unique_ptr<ParseNode const> uniqueNode(ownNode ? node : nullptr);

	std::vector<String> qualifiedName;
	if (filepath.empty()) {
		qualifiedName = GetCurrentDefQualifiedName();
		qualifiedName.push_back(node->GetToken(0)->_str);
	} else {
		qualifiedName = _compiler->GetQualifiedNameForFilePath(filepath);
		ASSERT(!qualifiedName.empty());
		if (qualifiedName.back() != node->GetToken(0)->_str)
			return Error(Err::ModuleNameMismatch, node->_filePos);
	}

	Error err = _compiler->GetOrCreateModule(qualifiedName, mod);
	if (err)
		return err;

	ASSERT(!ModuleDefinitionsLoaded(mod));

	mod->_moduleNode = std::move(uniqueNode);

	err = mod->Init(node);
	if (err)
		return err;

	AddNodeDef(node, mod);
	_currentDef.push_back(mod);
	ScopeGuard popDef([&] { _currentDef.pop_back(); });

	for (int32_t i = 1; i < node->_content.size(); ++i) {
		ParseNode const *sub = node->GetSubnode(i);
		err = AnnotateDefinition(sub);
		if (err)
			return err;
	}

	err = ResolveRecordedImports();
	if (err)
		return err;

	return Error();
}

Error Ast2Ir::AnnotateDefinition(ParseNode const *node)
{
	Error err;
	if (node->_label == "const" || node->_label == "var") {
		err = AnnotateVar(node);
	} else if (node->_label == "func") {
		err = AnnotateFunc(node);
	} else if (node->_label == "module") {
		Module *mod = nullptr;
		err = AnnotateModule(node, "", mod);
	} else if (node->_label == "import") {
		err = AnnotateImport(node);
	} else {
		err = Error(Err::UnexpectedDefinition, node->_filePos);
	}
	if (err)
		return err;

	return Error();
}

Error Ast2Ir::AnnotateImport(ParseNode const *node)
{
	ASSERT(node->_label == "import");
	ASSERT(node->GetContentSize() > 0);

	_importsToResolve.push_back(node);

	return Error();
}

Error Ast2Ir::AnnotateVar(ParseNode const *node)
{
	return Error();
}

Error Ast2Ir::AnnotateFunc(ParseNode const *node)
{
	return Error();
}

Error Ast2Ir::ResolveName(std::vector<String> &qualifiedName, Definition *&def, PosInFile const &filePos)
{
	ASSERT(def == nullptr);

	def = _compiler->GetRegisteredDefinition(qualifiedName);
	if (def)
		return Error();

	std::vector<String> suffixes;
	while (qualifiedName.size() > 0) {
		// find topmost module file that exists, and load it
		// it's the only one that can contain the (nested) definition
		String modFile = _compiler->GetFilePathForModule(qualifiedName);
		if (!modFile.empty()) {
			Module *mod = _compiler->GetRegisteredDefinition<Module>(qualifiedName);
			if (mod && ModuleDefinitionsLoaded(mod)) {
				// module exists and is loaded, but we already know the definition wasn't found in loaded data
				return Error(Err::ImportNotFound, filePos);
			}

			Error err = LoadModuleDefinitions(modFile, mod);
			if (err)
				return err;
			if (!suffixes.size()) {
				def = mod;
			} else {
				std::reverse(suffixes.begin(), suffixes.end());
				def = mod->GetDefinition(suffixes);
			}
			if (!def)
				return Error(Err::ImportNotFound, filePos);

			qualifiedName.insert(qualifiedName.end(), suffixes.begin(), suffixes.end());
			_currentDef.back()->NotifyImport(def);

			return Error();
		}

		suffixes.push_back(qualifiedName.back());
		qualifiedName.pop_back();
	}

	return Error(Err::ImportNotFound, filePos);
}

Error Ast2Ir::ResolveImport(ParseNode const *node)
{
	ASSERT(node->_label == "import");
	for (int i = 0; i < node->GetContentSize(); ++i) {
		std::vector<String> importName = ReadQualifiedName(node, i);
		ASSERT(importName.size() > 0);
		Definition *def = nullptr;
		Error err = ResolveName(importName, def, node->GetContentFilePos(i));
		if (err)
			return err;
	}

	return Error();
}

Error Ast2Ir::ResolveRecordedImports()
{
	auto imports = std::move(_importsToResolve);
	for (ParseNode const *impNode : imports) {
		Error err = ResolveImport(impNode);
		if (err)
			return err;
	}
	return Error();
}

std::vector<String> Ast2Ir::ReadQualifiedName(ParseNode const *parent, int contentIndex)
{
	std::vector<String> qualifiedName;
	ParseNode const *node = parent->GetSubnode(contentIndex);
	if (node) {
		ASSERT(node->_label == ".");
		for (int i = 0; i < node->GetContentSize(); ++i) {
			Token const *tok = node->GetToken(i);
			ASSERT(tok && tok->_type == Token::Type::Identifier);
			qualifiedName.push_back(tok->_str);
		}
	} else {
		Token const *ident = parent->GetToken(contentIndex);
		ASSERT(ident && ident->_type == Token::Type::Identifier);
		qualifiedName.push_back(ident->_str);
	}
	return qualifiedName;
}

String Ast2Ir::GetQualifiedNameStr(std::vector<String> const &qualifiedName)
{
	ASSERT(qualifiedName.size() > 0);
	String name(qualifiedName[0]);
	for (int i = 1; i < qualifiedName.size(); ++i)
		name += "." + qualifiedName[i];
	return name;
}

}