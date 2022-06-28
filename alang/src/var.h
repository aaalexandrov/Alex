#pragma once

#include "types.h"

namespace alang {

struct VarDef {
	String _name;
	Module *_module = nullptr;
	TypeDesc *_type = nullptr;
	Value _value;
};


}