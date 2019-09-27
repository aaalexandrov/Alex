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
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) ^= ((std::underlying_type<ENUMTYPE>::type)b)); } 

template <typename SRC, typename DST>
struct ValueRemapper {
  std::unordered_map<SRC, DST> _src2dst;
  std::unordered_map<DST, SRC> _dst2src;

  ValueRemapper(std::vector<std::pair<SRC, DST>> const &pairs) 
  {
    for (auto &pair : pairs) {
      _src2dst.insert(pair);
      _dst2src.insert(std::make_pair(pair.second, pair.first));
    }
  }

  DST ToDst(SRC src) 
  {
    return _src2dst.at(src);
  }

  SRC ToSrc(DST dst)
  {
    return _dst2src.at(dst);
  }
};

template <typename Enum>
Enum EnumInc(Enum e, std::underlying_type_t<Enum> inc = 1)
{
  return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(e) + inc);
}

NAMESPACE_END(util)