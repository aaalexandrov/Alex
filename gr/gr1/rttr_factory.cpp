#include "rttr_factory.h"

NAMESPACE_BEGIN(gr1)

rttr::type RttrFactory::GetDescendantType(rttr::type baseType)
{
	auto derived = baseType.get_derived_classes();
	auto found = std::find_if(derived.begin(), derived.end(), [&](rttr::type type) {
		return type.get_metadata(_discriminatorKey) == _discriminatorValue;
	});
	if (found != derived.end())
		return *found;
	return rttr::type::get<void>();
}

NAMESPACE_END(gr1)