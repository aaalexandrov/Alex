#include "module.h"
#include "core.h"

namespace alang {

Import::Import(Module *mod) 
	: _module(mod) 
{
}


Module::Module(String name)
	: Definition{ name }
{
}

Module::Module(ParseNode const *node)
	: Definition{ node }
{
	_imports.emplace_back(CoreModule.get());
}


Error Module::Analyze()
{
	return Error();
}

void Module::RegisterDefinition(std::unique_ptr<Definition> &&def)
{
	auto name = def->_name;
	def->_parentModule = this;
	_definitions.insert({ name, std::move(def) });
}

Definition *Module::GetDefinition(String name)
{
	
	return nullptr;
}

}