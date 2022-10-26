#pragma once

#include "types.h"

namespace alang {

struct VarDef : public Definition {
	TypeDesc *_type = nullptr;
	Value _value;
	bool _const = false;

	VarDef(String _name, TypeDesc *type, bool isConst);
};


}