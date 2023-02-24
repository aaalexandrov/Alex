#pragma once

#include "types.h"

namespace alang {

struct VarDef : public Definition {
	TypeDesc *_type = nullptr;
	Value _value;
	bool _const = false;

	VarDef(ParseNode const *node);

	Error Analyze() override;
};


}