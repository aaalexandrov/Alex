#pragma once

#include "common.h"
#include "definitions.h"
#include "ir.h"
#include <unordered_map>
#include <unordered_set>

namespace alang {

struct FuncData;
struct Module : public Definition {

	std::unique_ptr<ParseNode const> _moduleNode;
	std::unordered_map<String, std::unique_ptr<Definition>> _definitions;
	std::unordered_set<Definition*> _imports;

	std::vector<uint8_t> _statics;
	std::vector<OpCode> _code;

	Module(String name);
	Module() = default;

	Error Init(ParseNode const *node) override;

	void NotifyImport(Definition *def) override;

	rtti::TypeInfo const *GetTypeInfo() const override;

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

	Definition *GetDefinition(std::vector<String> const &qualifiedName);
	template <typename Def>
	Def *GetDefinition(std::vector<String> const &qualifiedName)
	{
		Definition *def = GetDefinition(qualifiedName);
		return rtti::Cast<Def>(def);
	}
};

}

namespace rtti {
template <> inline TypeInfo const *Get<alang::Module>() { return GetBases<alang::Module, alang::Definition>(); }
}