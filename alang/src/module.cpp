#include "module.h"

namespace alang {



Module::Module(String name, Module *parent)
	: _name{ name }
	, _parent{ parent }
{
}

void Module::RegisterType(std::unique_ptr<TypeDesc> &&type)
{
	auto name = type->_name;
	type->_module = this;
	_types.insert({ name, std::move(type) });
}

}