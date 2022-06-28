#include "module.h"

namespace alang {



Module::Module(String name, Module *parent)
	: _name{ name }
	, _parent{ parent }
{
}

void Module::RegisterVar(std::unique_ptr<VarDef> &&var)
{
	auto name = var->_name;
	var->_module = this;
	_definitions.insert({ name, std::move(var) });
}

void Module::RegisterType(std::unique_ptr<TypeDesc> &&type)
{
	auto name = type->_name;
	type->_module = this;
	_definitions.insert({ name, std::move(type) });
}

void Module::RegisterSubmodule(std::unique_ptr<Module> &&submod)
{
	auto name = submod->_name;
	submod->_parent = this;
	_definitions.insert({ name, std::move(submod) });
}

}