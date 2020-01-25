#include "type_factory.h"

NAMESPACE_BEGIN(util)

void TypeFactory::AddCreatedType(TypeInfo const *type)
{
	ASSERT(std::find(_createdTypes.begin(), _createdTypes.end(), type) == _createdTypes.end());
	_createdTypes.push_back(type);
}

TypeInfo const *TypeFactory::GetDescendantType(TypeInfo const *baseType)
{
	for (auto type : _createdTypes) {
		if (baseType->IsSameOrBase(type))
			return type;
	}
	return nullptr;
}


NAMESPACE_END(util)
