#include "common.h"
#include "module.h"
#include "parse.h"

namespace alang {

String PosInFile::ToString() const
{
	return _fileName + ":" + std::to_string(_line) + ":" + std::to_string(_posOnLine);
}

String Error::ToString() const
{
	return _location.ToString() + " - " + _error;
}

Definition::Definition(String name) 
	: _name(name)
{
}

Definition::Definition(ParseNode const *node)
	: _name(node->GetToken(0)->_str)
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