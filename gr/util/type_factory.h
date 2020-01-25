#pragma once

#include "rtti.h"
#include <vector>

NAMESPACE_BEGIN(util)

struct TypeFactory {

	void AddCreatedType(TypeInfo const *type);

	TypeInfo const *GetDescendantType(TypeInfo const *baseType);

	template<typename BaseType, typename... Args>
	BaseType *CreateInstance(TypeInfo const *baseType, Args... args)
	{
		if (!baseType)
			baseType = TypeInfo::Get<BaseType>();
		TypeInfo const *createdType = GetDescendantType(baseType);
		return createdType->GetConstructor()->Call<BaseType*>(std::forward(args)...);
	}

	template<typename BaseType, typename... Args>
	std::shared_ptr<BaseType> CreateInstanceShared(TypeInfo const *baseType, Args... args)
	{
		return std::shared_ptr<BaseType>(CreateInstance<BaseType>(baseType, std::forward(args)...));
	}
	
	std::vector<TypeInfo const *> _createdTypes;
};

NAMESPACE_END(util)