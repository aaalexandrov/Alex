#include "func.h"

namespace alang {

FuncData::FuncData(String name, ParseNode const *node, TypeDesc *signature)
	: Definition(name, node)
	, _signature(signature)
{
}

Func::Func(String name, ParseNode const *node, TypeDesc *signature)
	: FuncData(name, node, signature)
{
}

FuncExternal::FuncExternal(String name, ParseNode const *node, TypeDesc *signature, FuncType externalFunc)
	: FuncData(name, node, signature)
	, _externalFunc(externalFunc)
{
}

}