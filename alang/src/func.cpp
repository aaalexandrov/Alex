#include "func.h"

namespace alang {

FuncData::FuncData(String name, TypeDesc *signature)
	: Definition(name)
	, _signature(signature)
{
}

Func::Func(String name, TypeDesc *signature)
	: FuncData(name, signature)
{
}

FuncExternal::FuncExternal(String name, TypeDesc *signature, FuncType externalFunc)
	: FuncData(name, signature)
	, _externalFunc(externalFunc)
{
}

}