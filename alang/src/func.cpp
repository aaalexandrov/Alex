#include "func.h"
#include "parse.h"
#include "types.h"
#include "error.h"

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

	if (_node->_label != "func") 
		return Error(Err::ExpectedFunc, _node->_filePos);



	return Error();
}

rtti::TypeInfo const *FuncData::GetTypeInfo() const
{
	return rtti::Get<FuncData>();
}

std::unique_ptr<TypeDesc> FuncData::GetSignatrueTypeDesc(ParseNode const *node)
{
	ASSERT(node->_label == "func");
	auto type = std::unique_ptr<TypeDesc>(new TypeDesc());

	// todo: fill in "Func" generic instance with params from the nodes

	return type;
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

