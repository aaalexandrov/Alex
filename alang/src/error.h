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
};

}