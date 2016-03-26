#ifndef __UTIL_H
#define __UTIL_H

#include "Base.h"
#include <math.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))

#define PI 3.14159265358979323846f

namespace Util {
  // Comparrison predicates
  template <class T>
  struct EqualV {
    static inline bool Eq(T t1, T t2) { return t1 == t2; }
  };

  template <class T>
  struct LessV {
    static inline bool Lt(T t1, T t2) { return t1 < t2; }
  };

  template <class T>
  struct LessEqualV {
    static inline bool LEq(T t1, T t2) { return t1 <= t2; }
  };

  template <class T>
  struct GreaterV {
    static inline bool Lt(T t1, T t2) { return t1 > t2; }
  };

  template <class T>
  struct GreaterEqualV {
    static inline bool LEq(T t1, T t2) { return t1 >= t2; }
  };

  template <class T>
  struct Equal {
    static inline bool Eq(const T &t1, const T &t2) { return t1 == t2; }
  };

  template <class T>
  struct Less {
    static inline bool Lt(const T &t1, const T &t2) { return t1 < t2; }
  };

  template <class T>
  struct LessEqual {
    static inline bool LEq(const T &t1, const T &t2) { return t1 <= t2; }
  };

  template <class T>
  struct Greater {
    static inline bool Lt(const T &t1, const T &t2) { return t1 > t2; }
  };

  template <class T>
  struct GreaterEqual {
    static inline bool LEq(const T &t1, const T &t2) { return t1 >= t2; }
  };

  // Hashing
  struct HashSize_TV {
    template <class T>
    static inline size_t Hash(T t) { return (size_t) t; }
  };

  struct HashSize_T {
    template <class T>
    static inline size_t Hash(const T &t) { return (size_t) t; }
  };

  // Convenience functions
  template <class T>
  inline T Min(T t1, T t2) { return t1 < t2 ? t1 : t2; }

  template <class T>
  inline T Max(T t1, T t2) { return t1 > t2 ? t1 : t2; }

  template <class T>
  inline T Bound(T t, T min, T max) { return Min(Max(t, min), max); }

  template <class T>
  inline void Swap(T &a, T &b) { T t = a; a = b; b = t; }

  template <class T>
  inline T AlignToMultiple(T value, T block) { if (value % block) return value + block - value % block; else return value; }

  template <class T>
  inline T Sqr(T a) { return a * a; }

  template <class T>
  inline T Sign(T a) { if (a > 0) return 1; if (a < 0) return -1; return 0; }

  template <class T, class S>
  inline T Lerp(T a, T b, S w) { return a + (b - a) * w; }

  template <class T>
  inline T SmoothStep(T a) { return a * a * (3 - 2 * a); } // SmoothStep(0) = 0, SmoothStep(1) = 1, derivatives at 0 and 1 are 0

  template <class T, class S>
  inline T SmoothLerp(T a, T b, S w) { return Lerp(a, b, SmoothStep(w)); }

  template <class T>
  inline int SolveQuadratic(T a, T b, T c, T &x1, T &x2); // Solve the equation ax^2+bx+c=0. Returns the number of real roots, and sets x1 and x2 to those roots, where x1 <= x2.

  inline float abs(float a) { return fabsf(a); }

  template <class T>
  inline bool IsEqual(T a, T b, T delta) { return abs(a - b) <= delta; }

  template <class T>
  T GetEqualDelta() { return (T) 0.005; }

  template <class T>
  inline T RotateBits(T t, int iBits); // iBits < 0 means rotate left

  template <class T>
  inline int MostSignificantBitSet(T a); // return the index of the most significant bit in the argument that is set to 1

  template <class T>
  inline int LeastSignificantBitSet(T a); // return the index of the least significant bit in the argument that is set to 1

	template <class T>
	inline T RoundUpToPow2Minus1(T a); // returns a number with the same most significant bit as the argument, and all less significant bits set to 1

  template <class T>
  inline T &IndexStride(T *pT, int iStride, int iIndex) { return *(T*)((uint8_t *) pT + iStride * iIndex); }

  template <class T>
  inline T const &IndexStride(T const *pT, int iStride, int iIndex) { return *(T*)((uint8_t *) pT + iStride * iIndex); }

  template <class T, class E, class P /*= Less<E>*/ >
  int BinSearch(const T &arr, int iCount, E elem);

  template <class T>
  void Reverse(T &arr, int iEnd, int iStart = 0);

  template <class T, class E, class P /*= Less<E>*/ >
  void InsertSort(T &arr, int iEnd, int iStart = 0);

  template <class T, class E, class P /*= Less<E>*/ >
  void QSort(T &arr, int iEnd, int iStart = 0);

  // Floating point constants & functions
  static const uint32_t s_dwQNAN = 0xffffffff, s_dwSNAN = 0xffbfffff, s_dwINFINITY = 0x7f800000, s_dwNEG_INFINITY = 0xff800000;
  static const float F_QNAN = *(float *) &s_dwQNAN,
                     F_SNAN = *(float *) &s_dwSNAN,
                     F_INFINITY = *(float *) &s_dwINFINITY,
                     F_NEG_INFINITY = *(float *) &s_dwNEG_INFINITY;

  inline bool IsEqual(float a, float b) { return IsEqual<float>(a, b, GetEqualDelta<float>()); }

  // Key-value pair
  template <class K, class V, class H = HashSize_T, class E = Equal<K>, class L = Less<K> >
  struct TKeyValue {
    K m_Key;
    V m_Val;

    TKeyValue(K key, V val): m_Key(key), m_Val(val) {}

    static inline size_t Hash(K key)                                            { return H::Hash(key);                          }
    static inline size_t Hash(TKeyValue const &kKeyVal)                         { return H::Hash(kKeyVal.m_Key);                }
    static inline bool Eq(TKeyValue const &kKeyVal0, TKeyValue const &kKeyVal1) { return E::Eq(kKeyVal0.m_Key, kKeyVal1.m_Key); }
    static inline bool Eq(K const &key, TKeyValue const &kKeyVal)               { return E::Eq(kKeyVal.m_Key, key);             }
    static inline bool Lt(TKeyValue const &kKeyVal0, TKeyValue const &kKeyVal1) { return L::Lt(kKeyVal0.m_Key, kKeyVal1.m_Key); }
    static inline bool Lt(K const &key, TKeyValue const &kKeyVal)               { return L::Lt(key, kKeyVal.m_Key);             }
    static inline bool Lt(TKeyValue const &kKeyVal, K const &key)               { return L::Lt(kKeyVal.m_Key, key);             }
    template <class K1>
    static inline size_t Hash(K1 key)                                           { return H::Hash(key);                          }
    template <class K1>
    static inline size_t Eq(K1 key, TKeyValue const &kKeyVal)                   { return E::Eq(key, kKeyVal.m_Key);             }
  };
};

// By default, key value pairs will use the same allocator as the value
template <class K, class V, class H, class E, class L>
struct TSpecifyAllocator<Util::TKeyValue<K, V, H, E, L> > { typedef typename TGetAlloc<V>::Type Type; };


// Implementation ----------------------------------------------------------

template <class T>
int Util::SolveQuadratic(T a, T b, T c, T &x1, T &x2)
{
  if (Util::IsEqual(a, 0)) {
    if (Util::IsEqual(b, 0))
      if (Util::IsEqual(c, 0))
        return -1;
      else
        return 0;
    x1 = x2 = -c / b;
    return 1;
  }
  T d = b * b - 4 * a * c;
  if (d < 0)
    return 0;
  if (Util::IsEqual(d, 0)) {
    x1 = x2 = -b / (2 * a);
    return 1;
  }
  x1 = (-b - sqrt(d)) / (2 * a);
  x2 = (-b + sqrt(d)) / (2 * a);
  return 2;
}

template <class T>
T Util::RotateBits(T t, int iBits)
{
  T tRes;
  if (iBits > 0) // Rotate left
    tRes = (t << iBits) | (t >> (sizeof(t) * 8 - iBits));
  else // Rotate right
    tRes = (t >> iBits) | (t << (sizeof(t) * 8 - iBits));
  return tRes;
}

template <class T>
int Util::MostSignificantBitSet(T a)
{
  T nMask, nPrevMask;
  int iMin, iMax, iShift;
  if (!a)
    return -1;
  iMin = 0;
  iMax = sizeof(T) * 8;
  nMask = ~(T) 0;
  while (1) {
    nPrevMask = nMask;
    iShift = (iMax - iMin) / 2;
    if (!iShift)
      break;
    nMask = (nMask << iShift) & nMask;
    if (a & nMask)
      iMin += iShift;
    else {
      iMax -= iShift;
      nMask = ~nMask & nPrevMask;
    }
  }
  return iMin;
}

template <class T>
int Util::LeastSignificantBitSet(T a)
{
  T nMask, nPrevMask;
  int iMin, iMax, iShift;
  if (!a)
    return -1;
  iMin = 0;
  iMax = sizeof(T) * 8;
  nMask = ~(T) 0;
  while (1) {
    nPrevMask = nMask;
    iShift = (iMax - iMin) / 2;
    if (!iShift)
      break;
    nMask = ~(nMask << iShift) & nMask;
    if (a & nMask)
      iMax -= iShift;
    else {
      iMin += iShift;
      nMask = ~nMask & nPrevMask;
    }
  }
  return iMin;
}

template <class T>
inline T Util::RoundUpToPow2Minus1(T a)
{
	int iShift = 1;
	while (1) {
		T nNext = a | (a >> iShift);
		if (nNext == a)
			return a;
		a = nNext;
		iShift *= 2;
	}
}

template <class T, class E, class P>
int Util::BinSearch(T const &arr, int iCount, E elem)
{
  int iLeft, iRight, iMid;
  iLeft = 0;
  iRight = iCount - 1;
  while (iLeft <= iRight) {
    iMid = (iLeft + iRight) / 2;
    if (P::Lt(arr[iMid], elem))
      iLeft = iMid + 1;
    else
      if (P::Lt(elem, arr[iMid]))
        iRight = iMid - 1;
      else // Found
        return iMid;
  }
  ASSERT(iLeft >= 0 && iLeft <= iCount);
  // Not found
  return -iLeft - 1;
}

template <class T>
void Util::Reverse(T &arr, int iEnd, int iStart)
{
  int i, j;
  i = iStart;
  j = iEnd - 1;
  while (i < j) {
    Swap(arr[i], arr[j]);
    i++;
    j--;
  }
}

template <class T, class E, class P>
void Util::InsertSort(T &arr, int iEnd, int iStart)
{
  int i, j;
  E elem;
  for (i = iStart + 1; i < iEnd; i++) {
    elem = arr[i];
    j = i - 1;
    while (j >= iStart && P::Lt(elem, arr[j])) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = elem;
  }
}

template <class T, class E, class P>
void Util::QSort(T &arr, int iEnd, int iStart)
{
  if (iEnd - iStart < 24) {
    InsertSort<T, E, P>(arr, iEnd, iStart);
    return;
  }
  int i, j;
  E elem = arr[(iStart + iEnd) / 2];
  i = iStart;
  j = iEnd - 1;
  while (i < j) {
    while (i < iEnd - 1 && !(P::Lt(elem, arr[i])))
      i++;
    while (iStart < j && !(P::Lt(arr[j], elem)))
      j--;
    if (i < j) {
      Swap(arr[i], arr[j]);
      i++;
      j--;
    }
  }
  if (i == iStart) {
    Swap(arr[i], arr[(iStart + iEnd) / 2]);
    i++;
  } else
    if (j == iEnd - 1) {
      Swap(arr[(iStart + iEnd) / 2], arr[j]);
      j--;
    }
  QSort<T, E, P>(arr, j + 1, iStart);
  QSort<T, E, P>(arr, iEnd, i);
}

#endif
