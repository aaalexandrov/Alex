#pragma once

#include <vector>
#include <typeinfo>
#include <functional>
#include <cstddef>
#include <memory>

namespace rtti {

struct TypeInfo;
inline bool IsSameType(TypeInfo const *t0, TypeInfo const *t1)
{
	return t0 == t1;
}

inline bool SameTypeInfos(TypeInfo const **t0, TypeInfo const **t1)
{
	if (t0 == t1)
		return true;
	if (!t0 || !t1)
		return false;
	while (*t0 && *t1) {
		if (!IsSameType(*t0, *t1))
			break;
		++t0;
		++t1;
	}
	return !*t0 && !*t1;
}

struct Func {
	template <typename R, typename... Tn>
	bool IsCompatible() const;

	template <typename R, typename... Tn>
	R Invoke(Tn... args) const;

	virtual TypeInfo const **GetSignatureTypes() const = 0;
};


struct TypeInfo {
	char const *_name;
	size_t _size, _align;
	std::vector<TypeInfo *> _bases;
	std::unique_ptr<Func> _constructor;
};

template <typename T>
TypeInfo const *Get()
{
	size_t size, align;
	if constexpr (std::is_same_v<T, void>) {
		size = 0;
		align = 0;
	} else {
		size = sizeof(T);
		align = alignof(T);
	}
	static TypeInfo typeInfo{ typeid(T).name(), size, align };
	return &typeInfo;
}

template <typename... Tn>
TypeInfo const **GetTypeInfos()
{
	static TypeInfo const *typeInfos[sizeof...(Tn) + 1] = { Get<Tn>()..., nullptr };
	return typeInfos;
}

template <typename R, typename... Tn>
struct FuncImpl final : public Func {
	std::function<R(Tn...)> _func;
	TypeInfo const **GetSignatureTypes() const override { return GetTypeInfos<R, Tn...>(); }
};

template <typename R, typename... Tn>
bool Func::IsCompatible() const
{
	TypeInfo const **funcInfos = GetSignatureTypes();
	TypeInfo const **invokeInfos = GetTypeInfos<R, Tn...>();
	return SameTypeInfos(funcInfos, invokeInfos);
}

template <typename R, typename... Tn>
R Func::Invoke(Tn... args) const
{
	if (!IsCompatible<R, Tn...>())
		return R();
	auto *impl = static_cast<FuncImpl<R, Tn...> const *>(this);
	if (!impl->_func)
		return R();
	return impl->_func(args...);
}


struct Any {
	virtual TypeInfo *GetTypeInfo() const = 0;
};

template <typename T>
struct AnyImpl : public Any {
	TypeInfo *GetTypeInfo() const override { return Get<T>(); };
};

}
