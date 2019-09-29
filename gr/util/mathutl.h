#pragma once

#include <algorithm>
#include <limits>
#include <sstream>
#include "namespace.h"
#include "glm/glm.hpp"

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

template <typename Val>
inline Val Min(Val a, Val b)
{
  return std::min(a, b);
}

template <typename Val>
inline Val Max(Val a, Val b)
{
  return std::max(a, b);
}


template <typename Vec>
struct VecTraits {};

template <int D, typename T, glm::qualifier Q>
struct VecTraits<glm::vec<D, T, Q>> { 
  static const int Length = D;
  static const glm::qualifier Precision = Q; 
};

template <typename Vec>
Vec VecApplyOp(Vec v, std::function<typename Vec::value_type(typename Vec::value_type)> op)
{
  Vec res;
  for (int d = 0; d < Vec::length(); ++d) 
    res[d] = op(v[d]);
  return res;
}

template <typename Vec, typename Op>
Vec VecCombineOp(Vec v0, Vec v1, Op op)
{
  Vec res;
  for (int d = 0; d < Vec::length(); ++d)
    res[d] = op(v0[d], v1[d]);
}

template <typename Vec, typename ResElem = typename Vec::value_type>
glm::vec<VecTraits<Vec>::Length, ResElem, VecTraits<Vec>::Precision> VecCombineTypedOp(Vec v0, Vec v1, std::function<ResElem(typename Vec::value_type, typename Vec::value_type)> op)
{
  glm::vec<VecTraits<Vec>::Length, ResElem, VecTraits<Vec>::Precision> res;
  for (int d = 0; d < Vec::length(); ++d) 
    res[d] = op(v0[d], v1[d]);
  return res;
}

template <typename Vec>
bool VecLess(Vec a, Vec b)
{
  bool res = a[0] < b[0];
  for (int d = 1; d < Vec::length(); ++d)
    res &= a[d] < b[d];
  return res;
}

template <typename Vec>
bool VecLessEq(Vec a, Vec b)
{
  bool res = a[0] <= b[0];
  for (int d = 1; d < Vec::length(); ++d)
    res &= a[d] <= b[d];
  return res;
}

template <typename Vec>
typename Vec::value_type VecDot(Vec v0, Vec v1)
{
  Vec::value_type res = v0[0] * v1[0];
  for (int d = 1; d < Vec::length(); ++d)
    res += v0[d] * v1[d];
  return res;
}

template <typename Vec>
Vec VecSubClamp0(Vec v0, Vec v1)
{
  Vec res;
  for (int d = 0; d < Vec::length(); ++d)
    res[d] = v0[d] > v1[d] ? v0[d] - v1[d] : 0;
  return res;
}

template <typename Vec>
Vec VecDecClamp0(Vec v0)
{
  return VecSubClamp0(v0, glm::one<Vec>());
}

template <typename Vec>
Vec VecMin(Vec v0, Vec v1)
{
  Vec res;
  for (int d = 0; d < Vec::length(); ++d)
    res[d] = std::min(v0[d], v1[d]);
  return res;
}

template <typename Vec>
Vec VecMax(Vec v0, Vec v1)
{
  Vec res;
  for (int d = 0; d < Vec::length(); ++d)
    res[d] = std::max(v0[d], v1[d]);
  return res;
}


NAMESPACE_END(util)