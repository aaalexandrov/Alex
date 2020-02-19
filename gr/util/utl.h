#pragma once

#include "namespace.h"

NAMESPACE_BEGIN(std)

template <typename ElemType, size_t Size>
struct hash<array<ElemType, Size>> {
	auto operator() (array<ElemType, Size> const &key) const 
	{
		std::hash<ElemType> hasher;
		size_t result = 0;
		for (size_t i = 0; i < Size; ++i) {
			result = result * 31 + hasher(key[i]);
		}
		return result;
	}
};

template <typename ElemType>
struct hash<vector<ElemType>> {
	auto operator() (vector<ElemType> const &key) const
	{
		std::hash<ElemType> hasher;
		size_t result = 0;
		for (size_t i = 0; i < key.size(); ++i) {
			result = result * 31 + hasher(key[i]);
		}
		return result;
	}
};

template <typename T1, typename T2>
struct hash<pair<T1, T2>> {
	auto operator() (pair<T1, T2> const &key) const
	{
		return hash<T1>()(key.first) * 31 + hash<T2>()(key.second);
	}
};


NAMESPACE_END(std)

NAMESPACE_BEGIN(util)

template <typename Type>
size_t GetHash(Type const &val, size_t prevHash = 0)
{
	return 31 * prevHash + std::hash<Type>()(val);
}

template <typename Container>
inline typename Container::mapped_type FindOrDefault(Container const &container, typename Container::key_type key, typename Container::mapped_type defaultValue)
{
  auto it = container.find(key);
  if (it == container.end())
    return defaultValue;
  return it->second;
}

template <typename Obj, typename PointedObj = Obj>
std::shared_ptr<PointedObj> SharedFromThis(Obj *obj)
{
  if (!obj)
    return std::shared_ptr<PointedObj>();
  return std::static_pointer_cast<PointedObj>(obj->shared_from_this());
}

NAMESPACE_END(util)