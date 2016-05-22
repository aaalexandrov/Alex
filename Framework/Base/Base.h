#ifndef __BASE_H
#define __BASE_H

#include "Debug.h"
#include <new>

#define ARRSIZE(ARR)      (sizeof(ARR) / sizeof((ARR)[0]))
#define SAFE_RELEASE(p)   if (p) { p->Release(); p = 0; }
#define SAFE_DELETE(p)    if (p) { DEL(p); p = 0; }

// Macros to concatenate template parameters containing a comma, use as ID(CONCAT(Part1, Part2)) in order to pass both parts as a single parameter
#define CONCAT(...) __VA_ARGS__
#define ID(x) x

#if defined(_WIN32) || defined(_WIN64)
  #define WINDOWS
#elif defined(__linux__)
  #define LINUX
#endif

#ifndef __cdecl
  #if !defined(_MSC_VER) && defined(WINDOWS)
    #define __cdecl __attribute__((__cdecl__))
  #else
    #define __cdecl
  #endif
#endif

#include <stdint.h>

typedef unsigned int     UINT;

// Plain Old Data (POD) detection

struct TTrue  { inline static bool Value() { return true; } };
struct TFalse { inline static bool Value() { return false; } };

template <class T0, class T1>
struct TSameType { typedef TFalse TValue; };

template <class T>
struct TSameType<T, T> { typedef TTrue TValue; };

template <class T>
struct TIsPOD {	typedef TFalse TValue; };

template <>
struct TIsPOD<bool> { typedef TTrue TValue; };

template <>
struct TIsPOD<signed char> { typedef TTrue TValue; };

template <>
struct TIsPOD<unsigned char> { typedef TTrue TValue; };

template <>
struct TIsPOD<wchar_t> { typedef TTrue TValue; };

template <>
struct TIsPOD<signed short> { typedef TTrue TValue; };

template <>
struct TIsPOD<unsigned short> { typedef TTrue TValue; };

template <>
struct TIsPOD<signed int> { typedef TTrue TValue; };

template <>
struct TIsPOD<unsigned int> { typedef TTrue TValue; };

template <>
struct TIsPOD<signed long> { typedef TTrue TValue; };

template <>
struct TIsPOD<unsigned long> { typedef TTrue TValue; };

template <>
struct TIsPOD<signed long long> { typedef TTrue TValue; };

template <>
struct TIsPOD<unsigned long long> { typedef TTrue TValue; };

template <>
struct TIsPOD<float> { typedef TTrue TValue; };

template <>
struct TIsPOD<double> { typedef TTrue TValue; };

template <>
struct TIsPOD<long double> { typedef TTrue TValue; };

template <class T>
struct TIsPOD<T*> { typedef TTrue TValue; };

template <class T, class P>
struct TConDestructorVal {};

template <class T>
struct TConDestructorVal<T, TFalse> {
	static void Construct(T *pT)                 { new (pT) T; }
	static void ConstructCopy(T *pT, T const &t) { new (pT) T(t); }
	static void Destroy(T *pT)                   { pT->~T(); }
	static void Construct(T *pT, UINT uiCount)   { while (uiCount--) { Construct(pT++); } }
	static void Destroy(T *pT, UINT uiCount)     { while (uiCount--) { Destroy(pT++); } }
};

template<class T>
struct TConDestructorVal<T, TTrue> {
	static void Construct(T *pT)                 {}
	static void ConstructCopy(T *pT, T const &t) { *pT = t; }
	static void Destroy(T *pT)                   {}
	static void Construct(T *pT, UINT uiCount)   {}
	static void Destroy(T *pT, UINT uiCount)     {}
};

template <class T>
struct TConDestructor: TConDestructorVal<T, typename TIsPOD<T>::TValue> {};

#endif
