#ifndef __BASE_H
#define __BASE_H

#include "Debug.h"
#include <new>

#define ARRSIZE(ARR)      (sizeof(ARR) / sizeof((ARR)[0]))
#define SAFE_RELEASE(p)   if (p) { p->Release(); p = 0; }
#define SAFE_DELETE(p)    if (p) { delete p; p = 0; }

// Macros to concatenate template parameters containing a comma, use as ID(CONCAT(Part1, Part2)) in order to pass both parts as a single parameter
#define CONCAT(...) __VA_ARGS__
#define ID(x) x

#if defined(_WIN32) || defined(_WIN64)
  #define WINDOWS
#endif

#include <stdint.h>

typedef unsigned int     UINT;

// Smart pointers
class CRefCount {
public:
	uint32_t m_dwCount;

	inline CRefCount()                           { m_dwCount = 0; }
	inline CRefCount(const CRefCount &kRefCount) { m_dwCount = 0; }
	inline ~CRefCount()                          { ASSERT(!m_dwCount); }

	inline uint32_t Get() const                  { return m_dwCount; }
	inline void     Inc()                        { ++m_dwCount; }
	inline void     Dec()                        { ASSERT(m_dwCount); --m_dwCount; }
};

#define DEFREFCOUNT                                                  \
  public:                                                            \
    mutable CRefCount m_RefCount;                                    \
    inline uint32_t GetRefCount() const { return m_RefCount.Get(); } \
    inline void     Acquire() const     { m_RefCount.Inc(); }        \
    inline void     Release() const     { m_RefCount.Dec(); if (!m_RefCount.Get()) delete this;  }

#define DEFREFCOUNT_DUMMY                             \
  public:                                             \
    inline uint32_t GetRefCount() const { return 0; } \
    inline void     Acquire() const {}                \
    inline void     Release() const { delete this; }

template <class T>
class CSmartPtr {
public:
  typedef T TElem;

	T *m_pPtr;

	explicit CSmartPtr(T *pPtr = 0): m_pPtr(pPtr) { Acquire(m_pPtr); }
  explicit CSmartPtr(const CSmartPtr &pPtr)     { m_pPtr = pPtr.m_pPtr; Acquire(m_pPtr); }
	~CSmartPtr()                                  { Release(m_pPtr); }

	static inline void Acquire(T *pPtr)           { if (pPtr) pPtr->Acquire(); }
	static inline void Release(T *pPtr)           { if (pPtr) pPtr->Release(); }

	inline operator T*()                          { return m_pPtr; }
	inline T &operator *()                        { return *m_pPtr; }
	inline T *operator ->()                       { return m_pPtr; }
 	inline operator const T*() const              { return m_pPtr; }
	inline const T &operator *() const            { return *m_pPtr; }
	inline const T *operator ->() const           { return m_pPtr; }

  inline CSmartPtr<T> &operator =(T *pPtr)      { Acquire(pPtr); Release(m_pPtr); m_pPtr = pPtr; return *this; }
  inline CSmartPtr<T> &operator =(const CSmartPtr &pPtr) { return operator =(pPtr.m_pPtr); }
};

template <class T>
class CPtrDeleter {
public:
  static void FreePtr(T *pPtr) { delete pPtr; }
};

template <class T>
class CPtrArrayDeleter {
public:
  static void FreePtr(T *pPtr) { delete [] pPtr; }
};

template <class T>
class CPtrReleaser {
public:
  static void FreePtr(T *pPtr) { if (pPtr) pPtr->Release(); }
};

template <class T, class D>
class CAutoFreePtr {
public:
  typedef T TElem;

	T *m_pPtr;

	explicit CAutoFreePtr(T *pPtr = 0): m_pPtr(pPtr) {}
	~CAutoFreePtr()                                  { D::FreePtr(m_pPtr); }

	inline operator T*()                             { return m_pPtr; }
	inline T &operator *()                           { return *m_pPtr; }
	inline T *operator ->()                          { return m_pPtr; }

  inline void ReleasePtr()                         { D::FreePtr(m_pPtr); m_pPtr = 0; }
};

template <class T>
class CAutoDeletePtr: public CAutoFreePtr<T, CPtrDeleter<T> > {
public:
  explicit CAutoDeletePtr(T *pPtr = 0): CAutoFreePtr<T, CPtrDeleter<T> >(pPtr) {}
};

template <class T>
class CAutoDeleteArrayPtr: public CAutoFreePtr<T, CPtrArrayDeleter<T> > {
public:
  explicit CAutoDeleteArrayPtr(T *pPtr = 0): CAutoFreePtr<T, CPtrArrayDeleter<T> >(pPtr) {}
};

template <class T>
class CAutoReleasePtr: public CAutoFreePtr<T, CPtrReleaser<T> > {
public:
  explicit CAutoReleasePtr(T *pPtr = 0): CAutoFreePtr<T, CPtrReleaser<T> >(pPtr) {}
};

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
