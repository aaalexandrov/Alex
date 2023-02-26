#pragma once

#include "types.h"

namespace alang {

struct VarDef : public Definition {
	TypeDesc *_type = nullptr;
	Value _value;
	bool _const = false;

	VarDef(ParseNode const *node);

	rtti::TypeInfo const *GetTypeInfo() const override;

	Error Analyze() override;
};


}

namespace rtti {
template <> inline TypeInfo const *Get<alang::VarDef>() { return GetBases<alang::VarDef, alang::Definition>(); }
}