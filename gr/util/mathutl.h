#pragma once

#include <algorithm>
#include <limits>
#include <sstream>
#include "namespace.h"

NAMESPACE_BEGIN(util)

template <typename N>
constexpr N Clamp(N min, N max, N n)
{
  return std::max(min, std::min(max, n));
}

template <typename N>
constexpr bool IsPowerOf2(N n) { return !(n & (n - 1)); }

template <typename N>
uint8_t CountSetBits(N n)
{
  uint8_t set = 0;
  while (n) {
    n &= n - 1;
    ++set;
  }
  return set;
}

template <class T>
class limits {
public:
  template<bool HAS_INFINITY>
  struct Limit_Impl;

  template<>
  struct Limit_Impl<false> {
    static constexpr T Min() { return std::numeric_limits<T>::lowest(); }
    static constexpr T Max() { return std::numeric_limits<T>::max(); }
  };

  template<>
  struct Limit_Impl<true> {
    static constexpr T Min() { return -std::numeric_limits<T>::infinity(); }
    static constexpr T Max() { return  std::numeric_limits<T>::infinity(); }
  };

  typedef Limit_Impl<std::numeric_limits<T>::has_infinity> Impl;

  static constexpr T Min() { return Impl::Min(); }
  static constexpr T Max() { return Impl::Max(); }
};

template<class V>
std::string ToString(V const &v)
{
  std::ostringstream stream;
  stream << "(";
  for (int i = 0; i < v.length(); ++i) {
    if (i > 0)
      stream << ",";
    stream << v[i];
  }
  stream << ")";
  return stream.str();
}

NAMESPACE_END(util)