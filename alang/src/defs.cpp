#include "defs.h"

#include "parse.h"

namespace alang {

Error Def::Init(ParseNode const *node)
{
	return Error();
}

Error TypeDef::Init(ParseNode const *node)
{
	if (Error err = Def::Init(node))
		return err;
	return Error();
}

Error FuncDef::Init(ParseNode const *node)
{
	if (Error err = Def::Init(node))
		return err;
	return Error();
}

Error ValueDef::Init(ParseNode const *node)
{
	if (Error err = Def::Init(node))
		return err;
	return Error();
}

Error ModuleDef::Init(ParseNode const *node)
{
	if (Error err = Def::Init(node))
		return err;
	return Error();
}

Error ModuleDef::RegisterDef(std::unique_ptr<Def> &&def)
{
	auto defPtr = def.get();
	_definitions.insert(std::pair(defPtr->_name, std::move(def)));
    return Error();
}

}