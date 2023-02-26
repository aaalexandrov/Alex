#include "var.h"

namespace alang {

VarDef::VarDef(ParseNode const *node)
	: Definition(node)
{
}

rtti::TypeInfo const *VarDef::GetTypeInfo() const
{
	return rtti::Get<VarDef>();
}

Error VarDef::Analyze()
{
	return Error();
}

}

namespace rtti {
template <> TypeInfo const *Get<alang::VarDef>() { return GetBases<alang::VarDef, alang::Definition>(); }
}