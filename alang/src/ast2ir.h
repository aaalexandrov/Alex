#pragma once

#include "parse.h"
#include "ir.h"

namespace alang {

struct Compiler;
struct Definition;
struct Module;
struct Ast2Ir {
	Compiler *_compiler;
	std::unordered_map<ParseNode const *, Definition *> _nodeDefs;
	std::vector<ParseNode const *> _importsToResolve;

	std::vector<Definition *> _currentDef;

	Ast2Ir(Compiler *compiler);

	Error LoadModuleDefinitions(String filepath, Module *&mod);
	bool ModuleDefinitionsLoaded(Module *mod);

	Error AnnotateDefinition(ParseNode const *node);

	Error AnnotateModule(ParseNode const *node, String filepath, Module *&mod);
	Error AnnotateModule(std::unique_ptr<ParseNode const> &&node, String filepath, Module *&mod);

	Error AnnotateModuleImpl(ParseNode const *node, bool ownNode, String filepath, Module *&mod);
	Error AnnotateImport(ParseNode const *node);
	Error AnnotateVar(ParseNode const *node);
	Error AnnotateFunc(ParseNode const *node);

	Error ResolveName(std::vector<String> &qualifiedName, Definition *&def, PosInFile const &filePos);
	Error ResolveImport(ParseNode const *node);
	Error ResolveRecordedImports();

	Error CompileModule(ParseNode const *node);
	Error CompileImport(ParseNode const *node);
	Error CompileVar(ParseNode const *node);
	Error CompileFunc(ParseNode const *node);

	Error CompileOperator(ParseNode const *node);
	Error CompileIf(ParseNode const *node);
	Error CompileWhile(ParseNode const *node);
	Error CompileReturn(ParseNode const *node);
	Error CompileAssign(ParseNode const *node);
	Error CompileCall(ParseNode const *node);

	Error CompileExpression(ParseNode const *node);


	void AddNodeDef(ParseNode const *node, Definition *def);
	std::vector<String> GetCurrentDefQualifiedName();

	static std::vector<String> ReadQualifiedName(ParseNode const *parent, int contentIndex);
	static String GetQualifiedNameStr(std::vector<String> const &qualifiedName);
};

}