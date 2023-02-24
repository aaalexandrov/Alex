#include "func.h"

namespace alang {

FuncData::FuncData(String name, TypeDesc *signature)
	: Definition(name)
	, _signature(signature)
{
}

FuncData::FuncData(ParseNode const *node)
	: Definition(node)
{
}


Func::Func(String name, TypeDesc *signature)
	: FuncData(name, signature)
{
}

Func::Func(ParseNode const *node)
	: FuncData(node)
{
}


FuncExternal::FuncExternal(String name, TypeDesc *signature, FuncType externalFunc)
	: FuncData(name, signature)
	, _externalFunc(externalFunc)
{
}

FuncExternal::FuncExternal(ParseNode const *node)
	: FuncData(node)
{
}


}