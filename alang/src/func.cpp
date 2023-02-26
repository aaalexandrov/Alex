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

rtti::TypeInfo const *FuncData::GetTypeInfo() const
{
	return rtti::Get<FuncData>();
}


Func::Func(String name, TypeDesc *signature)
	: FuncData(name, signature)
{
}

Func::Func(ParseNode const *node)
	: FuncData(node)
{
}

rtti::TypeInfo const *Func::GetTypeInfo() const
{
	return rtti::Get<Func>();
}

Error Func::Analyze()
{
	return Error();
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

rtti::TypeInfo const *FuncExternal::GetTypeInfo() const
{ 
	return rtti::Get<FuncExternal>(); 
}

Error FuncExternal::Analyze()
{
	return Error();
}

}

namespace rtti {
template <> TypeInfo const *Get<alang::FuncData>() { return GetBases<alang::FuncData, alang::Definition>(); }
template <> TypeInfo const *Get<alang::Func>() { return GetBases<alang::Func, alang::FuncData>(); }
template <> TypeInfo const *Get<alang::FuncExternal>() { return GetBases<alang::FuncExternal, alang::FuncData>(); }
}