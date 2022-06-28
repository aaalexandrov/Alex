#pragma once

#include "common.h"
#include "types.h"
#include "var.h"
#include <unordered_map>

namespace alang {

struct Module {
	String _name;
	Module *_parent = nullptr;

	using Definition = std::variant<
		std::unique_ptr<VarDef>, 
		std::unique_ptr<TypeDesc>, 
		std::unique_ptr<Module>>;

	std::unordered_map<String, Definition> _definitions;

	Module(String name, Module *parent = nullptr);

	void RegisterVar(std::unique_ptr<VarDef> &&var);
	void RegisterType(std::unique_ptr<TypeDesc> &&type);
	void RegisterSubmodule(std::unique_ptr<Module> &&submod);
};

}