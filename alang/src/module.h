#pragma once

#include "common.h"
#include "types.h"
#include "var.h"
#include "func.h"
#include <unordered_map>

namespace alang {

struct Module;
struct Import {
	std::vector<String> _qualifiedName;
	Module *_module = nullptr;

	Import() = default;
	Import(String name) : _qualifiedName{ {name} } {}
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
};

}