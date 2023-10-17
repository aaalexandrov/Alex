#include "var.h"
#include "parse.h"
#include "error.h"

namespace alang {

Error VarDef::Init(ParseNode const *node)
{
	Error err = Definition::Init(node);
	if (err)
		return err;

	if (_node->_label != "const" && _node->_label != "var")
		return Error(Err::ExpectedVarOrConst, _node->_filePos);

	ParseNode const *assign = _node->GetSubnode(0);
	if (!assign || assign->_label != "=")
		return Error(Err::ExpectedAssign, _node->_filePos);

	ParseNode const *ofType = assign->GetSubnode(0);
	if (!ofType || ofType->_label != ":")
		return Error(Err::ExpectedOfType, _node->_filePos);

	_name = ofType->GetToken(0)->_str;
	_const = _node->_label == "const";

	return Error();
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

