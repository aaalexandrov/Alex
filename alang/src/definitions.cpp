#include "definitions.h"
#include "parse.h"
#include "module.h"
#include "error.h"

namespace alang {

Definition::Definition(String name)
	: _name(name)
{
}

Error Definition::Init(ParseNode const *node)
{
	ASSERT(node);
	_node = node;
	return Error();
}

String Definition::GetQualifiedName() const
{
	String qualified = _name;
	for (Definition *def = _parentDefinition; def; def = def->_parentDefinition)
		qualified = def->_name + "." + qualified;
	return qualified;
}

void Definition::GetQualifiedName(std::vector<String> &name) const
{
	if (_parentDefinition)
		_parentDefinition->GetQualifiedName(name);
	name.push_back(_name);
}

rtti::TypeInfo const *Definition::GetTypeInfo() const
{
	return rtti::Get<Definition>();
}


}