#pragma once

#include "util/namespace.h"
#include "rttr/type.h"

NAMESPACE_BEGIN(gr1)

struct RttrFactory {
	RttrFactory(rttr::variant discriminatorKey, rttr::variant discriminatorValue)
		: _discriminatorKey(discriminatorKey), _discriminatorValue(discriminatorValue) {}

	rttr::type GetDescendantType(rttr::type baseType);

	template<typename BaseType, typename... Args>
	BaseType *CreateInstance(rttr::type baseType, Args... args)
	{
		rttr::type createdType = GetDescendantType(baseType);
		if (createdType == rttr::type::get<void>())
			return nullptr;
		rttr::variant inst = createdType.create({ std::forward<Args>(args)... });
		return inst.get_value<BaseType*>();
	}

	template<typename BaseType, typename... Args>
	std::shared_ptr<BaseType> CreateInstanceShared(rttr::type baseType, Args... args)
	{
		return std::shared_ptr<BaseType>(CreateInstance<BaseType, Args...>(baseType, std::forward<Args>(args)...));
	}

	rttr::variant _discriminatorKey, _discriminatorValue;
};


NAMESPACE_END(gr1)