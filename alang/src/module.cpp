#include "module.h"
#include "core.h"

namespace alang {

Import::Import(String name) 
	: _qualifiedName(name) 
{
}

Import::Import(Module *mod) 
	: _qualifiedName(mod->GetQualifiedName())
	, _module(mod) 
{
}


Module::Module(String name, ParseNode const *node)
	: Definition{ name, node }
{
	_imports.emplace_back(CoreModule.get());
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