#include "module.h"

namespace alang {

Module::Module(String name)
	: Definition{ name }
{
}

void Module::RegisterDefinition(std::unique_ptr<Definition> &&def)
{
	auto name = def->_name;
	def->_parentModule = this;
	_definitions.insert({ name, std::move(def) });
}

}