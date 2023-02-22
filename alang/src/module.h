#pragma once

#include "common.h"
#include "types.h"
#include "var.h"
#include "func.h"
#include <unordered_map>

namespace alang {

struct Module;
struct Import {
	String _qualifiedName;
	Module *_module = nullptr;

	Import() = default;
	Import(String name);
	Import(Module *mod);
};

struct Module : public Definition {

	std::unordered_map<String, std::unique_ptr<Definition>> _definitions;
	std::vector<Import> _imports;

	Module(String name, ParseNode const *node);

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
		ASSERT(!def || dynamic_cast<Def*>(def));
		return static_cast<Def*>(def);
	}
};

}