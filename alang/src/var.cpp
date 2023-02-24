#include "var.h"

namespace alang {

VarDef::VarDef(ParseNode const *node)
	: Definition(node)
{
}

Error VarDef::Analyze()
{
	return Error();
}

}