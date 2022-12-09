#pragma once

#include <type_traits>
#include <vector>
#include "namespace.h"

NAMESPACE_BEGIN(util)

#define DEFINE_ENUM_BIT_OPERATORS(ENUMTYPE) \
inline constexpr ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) throw() { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) | ((std::underlying_type<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) |= ((std::underlying_type<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) throw() { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) & ((std::underlying_type<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) &= ((std::underlying_type<ENUMTYPE>::type)b)); } \
inline constexpr ENUMTYPE operator ~ (ENUMTYPE a) throw() { return ENUMTYPE(~((std::underlying_type<ENUMTYPE>::type)a)); } \
inline constexpr ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) throw() { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) ^ ((std::underlying_type<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) ^= ((std::underlying_type<ENUMTYPE>::type)b)); } \
inline constexpr bool operator !(ENUMTYPE a) throw() { return !(std::underlying_type<ENUMTYPE>::type)a; }

template<typename SRCARG, typename DSTARG>
DSTARG CheckBitmask(SRCARG src, std::unordered_map<std::underlying_type_t<SRCARG>, DSTARG> const &src2dst)
{
	std::underlying_type_t<DSTARG> dstVal = 0;
	std::underlying_type_t<SRCARG> srcVal = (std::underlying_type_t<SRCARG>)src;
	for (auto &pair : src2dst) {
		if ((pair.first & srcVal) == pair.first) {
			dstVal |= (std::underlying_type_t<DSTARG>)pair.second;
		}
	}
	return DSTARG(dstVal);
}

template <typename SRC, typename DST, bool BITMASK = false>
struct ValueRemapper {
  std::unordered_map<std::underlying_type_t<SRC>, DST> _src2dst;
  std::unordered_map<std::underlying_type_t<DST>, SRC> _dst2src;

  ValueRemapper(std::initializer_list<std::pair<SRC, DST>> const &pairs)
  {
    for (auto &pair : pairs) {
      _src2dst.insert(std::make_pair((std::underlying_type_t<SRC>)pair.first, pair.second));
      _dst2src.insert(std::make_pair((std::underlying_type_t<DST>)pair.second, pair.first));
    }
  }

  DST ToDst(SRC src) 
  {
		if (BITMASK) {
			return CheckBitmask(src, _src2dst);
		}

    return _src2dst.at((std::underlying_type_t<SRC>)src);
  }

  DST ToDst(SRC src, DST dflt)
  {
	  static_assert(!BITMASK);
	  auto it = _src2dst.find((std::underlying_type_t<SRC>)src);
	  return it != _src2dst.end() ? it->second : dflt;
  }

  SRC ToSrc(DST dst)
  {
		if (BITMASK) {
			return CheckBitmask(dst, _dst2src);
		}

    return _dst2src.at((std::underlying_type_t<DST>)dst);
  }

  SRC ToSrc(DST dst, SRC dflt)
  {
	  static_assert(!BITMASK);
	  auto it = _dst2src.find((std::underlying_type_t<DST>)dst);
	  return it != _dst2src.end() ? it->second : dflt;
  }
};

template <typename Enum>
Enum EnumInc(Enum e, std::underlying_type_t<Enum> inc = 1)
{
  return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(e) + inc);
}

NAMESPACE_END(util)