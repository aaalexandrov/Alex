#pragma once

#include <algorithm>
#include <limits>
#include <sstream>
#include "namespace.h"

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_projection.hpp"

NAMESPACE_BEGIN(util)

template <typename N>
constexpr N Clamp(N min, N max, N n)
{
  return std::max(min, std::min(max, n));
}

template <typename N>
constexpr int SignOfDifference(N n0, N n1)
{
	return n0 < n1 ? -1 : (n0 > n1 ? 1 : 0);
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

inline uint32_t RotateBitsLeft(uint32_t n, uint32_t rotateBy, uint32_t numBits = sizeof(uint32_t) * 8)
{
	uint32_t mask = (1u << (numBits + 1)) - 1u;
	return ((n << rotateBy) & mask) | ((n & mask) >> (numBits - rotateBy));
}

inline uint32_t RotateBitsRight(uint32_t n, uint32_t rotateBy, uint32_t numBits = sizeof(uint32_t) * 8)
{
	return RotateBitsLeft(n, numBits - rotateBy, numBits);
}

template <typename N>
N Sqr(N n) 
{ 
	return n * n; 
}

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

template <typename NumberType, typename Enable = void>
struct NumericTraits {
	using Num = NumberType;
	static constexpr Num Eps = Num(0);
	static constexpr Num Min = std::numeric_limits<Num>::lowest();
	static constexpr Num Max = std::numeric_limits<Num>::max();
};

template <typename NumberType>
struct NumericTraits<NumberType, std::enable_if_t<std::numeric_limits<NumberType>::is_iec559>> {
	using Num = NumberType;
	static constexpr float Eps = 1.e-6f;
	static constexpr float Min = -std::numeric_limits<float>::infinity();
	static constexpr float Max = std::numeric_limits<float>::infinity();
};

template <typename Num>
constexpr bool IsZero(Num n1, Num eps = NumericTraits<Num>::Eps) 
{  
	return -eps <= n1 && n1 <= eps;
}

template <int D, typename T, glm::qualifier Q>
constexpr bool IsZero(glm::vec<D, T, Q> const &v1, T eps = NumericTraits<T>::Eps)
{
	return glm::all(glm::lessThanEqual(glm::abs(v1), glm::vec<D, T, Q>(eps)));
}

template <typename Num>
constexpr bool IsEqual(Num n1, Num n2, Num eps = NumericTraits<Num>::Eps)
{
	return IsZero(n1 - n2, eps);
}

template <int D, typename T, glm::qualifier Q>
constexpr bool IsEqual(glm::vec<D, T, Q> const &v1, glm::vec<D, T, Q> const &v2, T eps = NumericTraits<T>::Eps)
{
	return IsZero(v1 - v2, eps);
}

template <typename T, glm::qualifier Q>
constexpr bool IsEqual(glm::qua<T, Q> const &q1, glm::qua<T, Q> const &q2, T eps = NumericTraits<T>::Eps)
{
	return glm::epsilonEqual(q1, q2, eps);
}

template <typename Num>
constexpr bool IsLess(Num n1, Num n2)
{
	return n1 < n2;
}

template <int D, typename T, glm::qualifier Q>
constexpr bool IsLess(glm::vec<D, T, Q> const &v1, glm::vec<D, T, Q> const &v2)
{
	return glm::all(glm::lessThan(v1, v2));
}

template <typename Num>
constexpr bool IsLessEqual(Num n1, Num n2)
{
	return n1 <= n2;
}

template <int D, typename T, glm::qualifier Q>
constexpr bool IsLessEqual(glm::vec<D, T, Q> const &v1, glm::vec<D, T, Q> const &v2)
{
	return glm::all(glm::lessThanEqual(v1, v2));
}

template <typename Num>
constexpr bool IsFinite(Num n) { return std::isfinite(n); }

template <int D, typename T, glm::qualifier Q>
constexpr bool IsFinite(glm::vec<D, T, Q> const &v) { return !glm::any(glm::isinf(v) || (glm::isnan(v))); }

template <typename Num>
constexpr bool IsInfinite(Num n) { return std::isinf(n); }

template <int D, typename T, glm::qualifier Q>
constexpr bool IsInfinite(glm::vec<D, T, Q> const &v) { return glm::any(glm::isinf(v)); }

template <typename Num>
constexpr bool IsNan(Num n) { return std::isnan(n); }

template <int D, typename T, glm::qualifier Q>
constexpr bool IsNan(glm::vec<D, T, Q> const &v) { return glm::any(glm::isnan(v)); }

template <typename Vec>
struct VecTraits {
	using ValueType = Vec;
	static constexpr int Length = 1;
	using ElemType = Vec;
	template <typename Elem>
	using WithElemType = Elem;
};

template <int D, typename T, glm::qualifier Q>
struct VecTraits<glm::vec<D, T, Q>> { 
	using ValueType = glm::vec<D, T, Q>;
	static constexpr int Length = D;
	static constexpr glm::qualifier Precision = Q; 
	using ElemType = T;
	template <typename Elem>
	using WithElemType = glm::vec<D, Elem, Q>;
};

template <typename Op, typename Vec, typename ResElem = typename Vec::value_type>
glm::vec<VecTraits<Vec>::Length, ResElem, VecTraits<Vec>::Precision> VecApplyOp(Vec v0, Op op)
{
	Vec res;
	for (int d = 0; d < Vec::length(); ++d)
		res[d] = op(v0[d]);
}

template <typename Op, typename Vec, typename ResElem = typename Vec::value_type>
glm::vec<VecTraits<Vec>::Length, ResElem, VecTraits<Vec>::Precision> VecCombineOp(Vec v0, Vec v1, Op op)
{
  Vec res;
  for (int d = 0; d < Vec::length(); ++d)
    res[d] = op(v0[d], v1[d]);
}

template <typename Vec>
typename Vec::value_type VecDot(Vec v0, Vec v1)
{
	typename Vec::value_type res = v0[0] * v1[0];
	for (int d = 1; d < Vec::length(); ++d)
		res += v0[d] * v1[d];
	return res;
}

template <typename Vec>
Vec VecNormalize(Vec const &v)
{
	return IsZero(v) ? v : glm::normalize(v);
}

template <typename Vec>
Vec VecCardinal(int dim, typename VecTraits<Vec>::ElemType e = 1)
{
	Vec v(0);
	v[dim] = e;
	return v;
}

template <typename Vec>
Vec VecFromMask(int mask)
{
	using Num = typename Vec::Num;
	Vec res;
	for (int d = 0; d < Vec::Dim; ++d) {
		res[d] = Num((mask >> d) & 1);
	}
	return res;
}

template <typename VecType>
struct Rotation {};

template <typename T, glm::qualifier Q>
struct Rotation<glm::vec<2, T, Q>> {
	using Vec = glm::vec<2, T, Q>;
	using Num = typename VecTraits<Vec>::ValueType;
	using Rot = Num;

	static constexpr Vec Rotate(Rot r, Vec const &v)
	{
		Num c = glm::cos(r), s = glm::sin(r);
		return Vec(c*v.x - s*v.y, s*v.x + c*v.y);
	}

	static constexpr Rot Normalize(Rot r) { return fmod(r, glm::pi<Num>()); }
	static constexpr Rot Compose(Rot r0, Rot r1) { return r0 + r1; }
	static constexpr Rot Inverse(Rot r) { return -r; }
	static constexpr Rot Identity() { return Rot(0); }
};

template <typename T, glm::qualifier Q>
struct Rotation<glm::vec<3, T, Q>> {
	using Vec = glm::vec<3, T, Q>;
	using Num = typename VecTraits<Vec>::ValueType;
	using Rot = glm::qua<T, Q>;

	static constexpr Vec Rotate(Rot const &r, Vec const &v) { return r * v; }
	static constexpr Rot Normalize(Rot const &r) { return glm::normalize(r); }
	static constexpr Rot Compose(Rot r0, Rot r1) { return r0 * r1; }
	static constexpr Rot Inverse(Rot r) { return glm::inverse(r); }
	static constexpr Rot Identity() { return glm::identity<Rot>(); }
};

template <typename Num>
int SolveQuadratic(Num a, Num b, Num c, Num &x0, Num &x1)
{
	if (IsZero(a)) {
		if (IsZero(b)) {
			if (IsZero(c)) {
				// 0*x=0, any x
				x0 = -std::numeric_limits<Num>::infinity();
				x1 = std::numeric_limits<Num>::infinity();
				return 2;
			}
			// 0*x = -c, c!= 0 - no solutions
			x0 = x1 = std::numeric_limits<Num>::quiet_nan();
			return 0;
		}
		x0 = x1 = -c / b;
		return 1;
	}

	Num d = b * b - 4 * a * c;
	if (d < 0) {
		// negative discriminant, no real solutions
		x0 = x1 = std::numeric_limits<Num>::quiet_nan();
		return 0;
	}
	d = sqrt(d);
	x0 = (-b - d) / 2 * a;
	x1 = (-b + d) / 2 * a;
	return 1 + !IsEqual(x0, x1);
}

NAMESPACE_END(util)

NAMESPACE_BEGIN(std)

template <int Dim, typename Num, glm::precision Prec>
struct hash<glm::vec<Dim, Num, Prec>> {
	using VecType = glm::vec<Dim, Num, Prec>;
	size_t operator()(VecType const &v) const
	{
		size_t hash = 0;
		std::hash<Num> hasher;
		for (int d = 0; d < Dim; ++d) {
			hash = 31 * hash + hasher(v[d]);

		}
		return hash;
	}
};

NAMESPACE_END(std)