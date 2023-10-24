#include "func.h"

namespace alang {

FuncData::FuncData(String name, TypeDesc *signature)
	: Definition(name)
	, _signature(signature)
{
}

Error FuncData::Init(ParseNode const *node)
{
	Error err = Definition::Init(node);
	if (err)
		return err;

	return Error();
}

rtti::TypeInfo const *FuncData::GetTypeInfo() const
{
	return rtti::Get<FuncData>();
}


Func::Func(String name, TypeDesc *signature)
	: FuncData(name, signature)
{
}

Error Func::Init(ParseNode const *node)
{
	Error err = FuncData::Init(node);
	if (err)
		return err;

	return Error();

}

rtti::TypeInfo const *Func::GetTypeInfo() const
{
	return rtti::Get<Func>();
}

FuncExternal::FuncExternal(String name, TypeDesc *signature, FuncType externalFunc)
	: FuncData(name, signature)
	, _externalFunc(externalFunc)
{
}

Error FuncExternal::Init(ParseNode const *node)
{
	Error err = FuncData::Init(node);
	if (err)
		return err;

	return Error();
}

rtti::TypeInfo const *FuncExternal::GetTypeInfo() const
{ 
	return rtti::Get<FuncExternal>(); 
}

}

