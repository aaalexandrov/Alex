#include "module.h"
#include "core.h"

namespace alang {

Module::Module(String name)
	: Definition{ name }
{
}

Module::Module(ParseNode const *node)
	: Definition{ node }
{
	_imports.emplace_back(CoreModule.get());
}

rtti::TypeInfo const *Module::GetTypeInfo() const
{
	return rtti::Get<Module>();
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
	auto it = _definitions.find(name);
	return it != _definitions.end() ? it->second.get() : nullptr;
}

}

