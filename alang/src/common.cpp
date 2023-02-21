#include "common.h"

namespace alang {

Definition::Definition(String name, ParseNode const *node) 
	: _name(name)
	, _node(node) 
{
}

}