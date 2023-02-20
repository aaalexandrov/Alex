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
	static inline String UnexpectedDefinition = "Unexpected definition";
	static inline String ExpectedImport = "Expected 'import'";
	static inline String ExpectedQualifiedName = "Expected qualified name";
	static inline String ExpectedFunc = "Expected 'func'";
};

}