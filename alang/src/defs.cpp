#include "defs.h"

#include "parse.h"
#include "compile.h"

namespace alang {

Error Def::Scan(Compiler *compiler, ParseNode const *node)
{
	ASSERT(_state < Scanned);
	_node = node;
	_state = Scanned;
	return Error();
}

Error TypeDef::Scan(Compiler *compiler, ParseNode const *node)
{
	if (Error err = Def::Scan(compiler, node))
		return err;
	return Error();
}

Error FuncDef::Scan(Compiler *compiler, ParseNode const *node)
{
	if (Error err = Def::Scan(compiler, node))
		return err;
	return Error();
}

Error ValueDef::Scan(Compiler *compiler, ParseNode const *node)
{
	if (Error err = Def::Scan(compiler, node))
		return err;
	return Error();
}

Error ModuleDef::Scan(Compiler *compiler, ParseNode const *node)
{
	if (Error err = Def::Scan(compiler, node))
		return err;

	for (int i = 1; i < node->GetContentSize(); ++i) {
		ParseNode const *sub = node->GetSubnode(i);
		ASSERT(sub);
		std::unique_ptr<Def> newDef;
		if (sub->_label == "const" || sub->_label == "var") {

			newDef = std::make_unique<ValueDef>(GetNodeName(sub->GetContent(0)), this);
		} else if (sub->_label == "func") {
			newDef = std::make_unique<FuncDef>(GetNodeName(sub->GetContent(0)), this);
		} else if (sub->_label == "module") {
			ModuleDef *subMod;
			compiler->ScanModule(this, sub, subMod);
		} else if (sub->_label == "import") {
			for (int i = 0; i < sub->GetContentSize(); ++i) {
				_imports.push_back(sub->GetContent(i));
			}
		} else {
			ASSERT(0);
		}
		if (newDef) {
			RegisterDef(std::move(newDef));
		}
	}

	return Error();
}

Error ModuleDef::RegisterDef(std::unique_ptr<Def> &&def)
{
	ASSERT(def);
	auto defPtr = def.get();
	_definitions.insert(std::pair(defPtr->_name, std::move(def)));
    return Error();
}

}