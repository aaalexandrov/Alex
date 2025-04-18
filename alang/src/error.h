#pragma once

#include "common.h"

namespace alang {

struct Err {
	// tokenizer errors
	static inline String InvalidUtf8 = "Invalid UTF - 8 input";
	static inline String NonClosedComment = "Non-closed comment";
	static inline String UnrecognizedSeparator = "Unrecognized separator";
	static inline String InvalidCharLiteral = "Invalid Char literal";
	static inline String InvalidStringLiteral = "Invalid String literal";

	// semantic analyzer errors
	static inline String ExpectedModule = "Expected 'module'";
	static inline String ImportNotFound = "Import not found";
	static inline String UnexpectedDefinition = "Unexpected definition";
	static inline String ExpectedImport = "Expected 'import'";
	static inline String ExpectedQualifiedName = "Expected qualified name";
	static inline String ExpectedFunc = "Expected 'func'";
	static inline String DefinitionNotModule = "Definition is not a module";
	static inline String ExpectedVarOrConst = "Expected 'var' or 'const'";
	static inline String ExpectedAssign = "Expected assignment";
	static inline String ExpectedOfType = "Expected typed value";
	static inline String ModuleNameMismatch = "Module name not matching the file name of the containing file";
	static inline String DuplicateDefinition = "Definition with that name already exists in this scope";
	static inline String UndefinedSymbol = "Symbol not defined";
	static inline String AmbiguousSymbolNotFound = "Symbol not found, possibly ambiguous imports";
	static inline String PathForModuleNotFound = "Path for imported module not found";
	static inline String UnexpectedDefinitionKind = "Definition exists but is not the expected kind";
	static inline String ExpectedTypeParameters = "Missing type parameters";
	static inline String UnexpectedTypeParameters = "Type parameters applied to a type that's parametric";
	static inline String ExpectedGenericDefinition = "Expected generic definition name";
	static inline String NumberOfParametersMismatch = "Mismatch in number of expected parameters";
	static inline String Unimplemented = "Unimplemented!";
	static inline String NotConst = "Not a constant value";
	static inline String UnexpectedLiteral = "Unexpected literal";
	static inline String MismatchingTypeForLiteral = "Mismatching type for literal value";
	static inline String ExpectedType = "Expected type name";
	static inline String MismatchingGenericArguments = "Mismatching generic arguments";
};

}