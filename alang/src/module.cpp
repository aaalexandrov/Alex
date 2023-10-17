#include "module.h"
#include "core.h"
#include "parse.h"

namespace alang {

Module::Module(String name)
	: Definition{ name }
{
}

Error Module::Init(ParseNode const *node)
{
	Error err = Definition::Init(node);
	if (err)
		return err;

	_name = _node->GetToken(0)->_str;
	_imports.emplace_back(CoreModule.get());

	return Error();
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
	def->_parentDefinition = this;
	_definitions.insert({ name, std::move(def) });
}

Definition *Module::GetDefinition(String name)
{
	auto it = _definitions.find(name);
	return it != _definitions.end() ? it->second.get() : nullptr;
}

}

