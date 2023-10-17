#pragma once

#include "common.h"

namespace alang {

struct Module;
struct ParseNode;
struct Definition : public rtti::Any {
	String _name;
	Definition *_parentDefinition = nullptr;
	ParseNode const *_node = nullptr;

	Definition(String name);
	Definition() = default;
	virtual ~Definition() {}

	virtual Error Init(ParseNode const *node);

	String GetQualifiedName() const;
	void GetQualifiedName(std::vector<String> &name) const;

	rtti::TypeInfo const *GetTypeInfo() const override;

	virtual Error Analyze() = 0;
};

}

