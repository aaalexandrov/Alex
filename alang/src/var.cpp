#include "var.h"

namespace alang {

VarDef::VarDef(String _name, TypeDesc *type, bool isConst)
	: Definition(_name)
	, _type(type)
	, _const(isConst)
{
}

}