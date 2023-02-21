#include "var.h"

namespace alang {

VarDef::VarDef(String name, ParseNode const *node, TypeDesc *type, bool isConst)
	: Definition(name, node)
	, _type(type)
	, _const(isConst)
{
}

}