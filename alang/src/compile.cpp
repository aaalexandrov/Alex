#include "compile.h"

namespace alang {

String Compiler::GetFilePathForModule(std::vector<String> const &qualifiedName)
{
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
	return Error();
}

Error Compiler::ScanModule(ParseNode const *mod)
{
	return Error();
}

Error Compiler::CompileFile(String filePath)
{
	std::unique_ptr<ParseNode> mod;
	Error err = ParseFile(filePath, mod);


}

}