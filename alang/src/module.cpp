#include "module.h"
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

	return Error();
}

void Module::NotifyImport(Definition *def)
{
	_imports.insert(def);
}

rtti::TypeInfo const *Module::GetTypeInfo() const
{
	return rtti::Get<Module>();
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

Definition *Module::GetDefinition(std::vector<String> const &qualifiedName)
{
	Module *parent = this;
	for (int i = 0; parent && i < qualifiedName.size(); ++i) {
		Definition *def = parent->GetDefinition(qualifiedName[i]);
		if (!def || i == qualifiedName.size() - 1)
			return def;
		parent = rtti::Cast<Module>(def);
	}
	return nullptr;
}

}

