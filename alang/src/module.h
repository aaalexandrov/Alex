#pragma once

#include "common.h"
#include "types.h"
#include "var.h"
#include "func.h"
#include <unordered_map>

namespace alang {

struct Module;
struct Module : public Definition {

	std::unordered_map<String, std::unique_ptr<Definition>> _definitions;
	std::vector<Module *> _imports;

	Module(String name);
	Module(ParseNode const *node);

	rtti::TypeInfo const *GetTypeInfo() const override;

	Error Analyze() override;

	void RegisterDefinition(std::unique_ptr<Definition> &&def);

	template <typename Def>
	void RegisterDefinition(std::unique_ptr<Def> &&def)
	{
		RegisterDefinition(std::unique_ptr<Definition>(def.release()));
	}

	Definition *GetDefinition(String name);

	template <typename Def>
	Def *GetDefinition(String name)
	{
		Definition *def = GetDefinition(name);
		return rtti::Cast<Def>(def);
	}
};

}

namespace rtti {
template <> inline TypeInfo const *Get<alang::Module>() { return GetBases<alang::Module, alang::Definition>(); }
}