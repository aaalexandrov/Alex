#include "common.h"
#include "module.h"

namespace alang {

Definition::Definition(String name, ParseNode const *node) 
	: _name(name)
	, _node(node) 
{
}

String Definition::GetQualifiedName() const
{
	String qualified = _name;
	for (Module *mod = _parentModule; mod; mod = mod->_parentModule)
		qualified = mod->_name + "." + qualified;
	return qualified;
}

}