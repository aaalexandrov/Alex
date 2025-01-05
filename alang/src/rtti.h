#pragma once

#include <vector>
#include <typeinfo>
#include <functional>
#include <cstddef>
#include <memory>
#include "dbg.h"

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

	virtual ~Func() {}
	virtual TypeInfo const **GetSignatureTypes() const = 0;
};


struct TypeInfo {
	char const *_name;
	size_t _size, _align;
	std::vector<TypeInfo const *> _bases;
	std::unique_ptr<Func> _constructor;
};

template <typename T>
TypeInfo const *GetDefault()
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

inline TypeInfo const *SetBasesConstructor(TypeInfo const *typeInfo, TypeInfo const **baseInfos, std::unique_ptr<Func> &&constructor = std::unique_ptr<Func>())
{
	TypeInfo *info = const_cast<TypeInfo *>(typeInfo);
	ASSERT(info->_bases.empty());
	ASSERT(!info->_constructor);
	if (baseInfos) {
		for (int i = 0; baseInfos[i]; ++i)
			info->_bases.push_back(baseInfos[i]);
	}
	info->_constructor = std::move(constructor);
	return info;
}

template <typename T>
TypeInfo const *Get()
{
	return GetDefault<T>();
}

template <typename... Tn>
TypeInfo const **GetTypeInfos()
{
	static TypeInfo const *typeInfos[sizeof...(Tn) + 1] = { Get<Tn>()..., nullptr };
	return typeInfos;
}

template <typename T, typename... Bases>
TypeInfo const *GetBases()
{
	static TypeInfo const *typeInfo = SetBasesConstructor(GetDefault<T>(), GetTypeInfos<Bases...>());
	return typeInfo;
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
	virtual TypeInfo const *GetTypeInfo() const = 0;
};

template <typename T>
bool IsType(Any *obj)
{
	return obj && IsSameType(obj->GetTypeInfo(), Get<T>());
}

inline bool CanCast(TypeInfo const *src, TypeInfo const *dst)
{
	ASSERT(src);
	ASSERT(dst);
	if (IsSameType(src, dst))
		return true;
	for (auto base : src->_bases) {
		if (CanCast(base, dst))
			return true;
	}
	return false;
}

template <typename T>
T *Cast(Any *obj)
{
	if (obj && CanCast(obj->GetTypeInfo(), Get<T>()))
		return static_cast<T *>(obj);
	return nullptr;
}

template <typename T>
T const *Cast(Any const *obj)
{
	return Cast(const_cast<T *>(obj));
}


}
