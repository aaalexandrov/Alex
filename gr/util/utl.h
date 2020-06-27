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

template <typename Vector>
void RemoveElement(Vector &v, int i)
{
	std::swap(v[i], v.back());
	v.resize(v.size() - 1);
}

template <typename Type>
size_t GetHash(Type const &val, size_t prevHash = 0)
{
	return 31 * prevHash + std::hash<Type>()(val);
}

template <typename HashValue, size_t BasisValue, size_t PrimeValue> struct HashFNVImpl {
	using HashType = HashValue;
	static const HashType BasisOffset = static_cast<HashType>(BasisValue);
	static const HashType Prime = static_cast<HashType>(PrimeValue);

	static HashType Fnv1(uint8_t const *data, size_t dataLength, HashType hash = BasisOffset)
	{
		for (size_t i = 0; i < dataLength; ++i) {
			hash = hash * Prime;
			hash = hash ^ data[i];
		}
		return hash;
	}

	static HashType Fnv1a(uint8_t const *data, size_t dataLength, HashType hash = BasisOffset)
	{
		for (size_t i = 0; i < dataLength; ++i) {
			hash = hash ^ data[i];
			hash = hash * Prime;
		}
		return hash;
	}

	template <typename Val, typename = std::enable_if_t<std::is_pod_v<Val> && !std::is_pointer_v<Val>>>
	static HashType Fnv1(Val const &v, HashType hash = BasisOffset) { return Fnv1(reinterpret_cast<uint8_t const *>(&v), sizeof(v), hash); }
	template <typename Val, typename = std::enable_if_t<std::is_pod_v<Val> && !std::is_pointer_v<Val>>>
	static HashType Fnv1a(Val const &v, HashType hash = BasisOffset) { return Fnv1a(reinterpret_cast<uint8_t const *>(&v), sizeof(v), hash); }

	template <typename Val, typename = std::enable_if_t<std::is_pod_v<Val>>>
	static HashType Fnv1(Val const *p, size_t elems, HashType hash = BasisOffset) { return Fnv1(reinterpret_cast<uint8_t const *>(p), elems * sizeof(Val), hash); }
	template <typename Val, typename = std::enable_if_t<std::is_pod_v<Val>>>
	static HashType Fnv1a(Val const *p, size_t elems, HashType hash = BasisOffset) { return Fnv1a(reinterpret_cast<uint8_t const *>(p), elems * sizeof(Val), hash); }

	static HashType Fnv1(std::string s, HashType hash = BasisOffset) { return Fnv1(s.c_str(), s.size(), hash); }
	static HashType Fnv1a(std::string s, HashType hash = BasisOffset) { return Fnv1a(s.c_str(), s.size(), hash); }

	template <typename Iterator>
	static HashType ForEachFnv1(Iterator begin, Iterator end, HashType hash = BasisOffset)
	{
		for (; begin != end; ++begin) {
			hash = Fnv1(*it, hash);
		}
		return hash;
	}

	template <typename Iterator>
	static HashType ForEachFnv1a(Iterator begin, Iterator end, HashType hash = BasisOffset)
	{
		for (; begin != end; ++begin) {
			hash = Fnv1a(*it, hash);
		}
		return hash;
	}
};

template <int Size> struct HashFNV {};
template <> struct HashFNV<32> : HashFNVImpl<uint32_t, 0x811c9dc5ull, 0x01000193ull> {};
template <> struct HashFNV<64> : HashFNVImpl<uint64_t, 0xcbf29ce484222325ull, 0x00000100000001B3ull> {};

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

#if defined(_MSC_VER)
#define DISABLE_WARNING_PUSH           __pragma(warning( push ))
#define DISABLE_WARNING_POP            __pragma(warning( pop )) 
#define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(4100)
#define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(4505)
#define DISABLE_WARNING_CONVERSION_TO_SMALLER_TYPE       DISABLE_WARNING(4267)
// other warnings you want to deactivate...

#elif defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop) 
#define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(-Wunused-parameter)
#define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(-Wunused-function)
#define DISABLE_WARNING_CONVERSION_TO_SMALLER_TYPE       DISABLE_WARNING(-Wconversion)
// other warnings you want to deactivate... 

#else
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#define DISABLE_WARNING_UNREFERENCED_FUNCTION
// other warnings you want to deactivate... 

#endif