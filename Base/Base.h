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

typedef unsigned int     UINT;

typedef unsigned char    BYTE;
typedef unsigned short   WORD;
typedef unsigned long    DWORD;
typedef unsigned __int64 QWORD;

// Smart pointers
class CRefCount {
public:
	DWORD m_dwCount;

	inline CRefCount()                           { m_dwCount = 0; }
	inline CRefCount(const CRefCount &kRefCount) { m_dwCount = 0; }
	inline ~CRefCount()                          { ASSERT(!m_dwCount); }

	inline DWORD Get()  { return m_dwCount; }
	inline void  Inc()  { ++m_dwCount; }
	inline void  Dec()  { ASSERT(m_dwCount); --m_dwCount; }
};

#define DEFREFCOUNT                                               \
  public:                                                         \
    mutable CRefCount m_RefCount;                                 \
    inline DWORD GetRefCount() const { return m_RefCount.Get(); } \
    inline void  Acquire() const { m_RefCount.Inc(); }            \
    inline void  Release() const { m_RefCount.Dec(); if (!m_RefCount.Get()) delete this;  }

#define DEFREFCOUNT_DUMMY                          \
  public:                                          \
    inline DWORD GetRefCount() const { return 0; } \
    inline void  Acquire() const {}                \
    inline void  Release() const { delete this; }

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
  explicit CAutoDeletePtr(T *pPtr = 0): CAutoFreePtr(pPtr) {}
};

template <class T>
class CAutoDeleteArrayPtr: public CAutoFreePtr<T, CPtrArrayDeleter<T> > {
public:
  explicit CAutoDeleteArrayPtr(T *pPtr = 0): CAutoFreePtr(pPtr) {}
};

template <class T>
class CAutoReleasePtr: public CAutoFreePtr<T, CPtrReleaser<T> > {
public:
  explicit CAutoReleasePtr(T *pPtr = 0): CAutoFreePtr(pPtr) {}
};

// Plain Old Data (POD) detection

struct TTrue  { inline static bool Value() { return true; } };
struct TFalse { inline static bool Value() { return false; } };

template <class T>
struct TIsPOD {
	typedef TFalse TValue;
};

template<>
struct TIsPOD<bool> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<signed char> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<unsigned char> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<wchar_t> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<signed short> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<unsigned short> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<signed int> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<unsigned int> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<signed long> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<unsigned long> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<signed __int64> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<unsigned __int64> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<float> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<double> {
	typedef TTrue TValue;
};

template<>
struct TIsPOD<long double> {
	typedef TTrue TValue;
};

template <class T>
struct TIsPOD<T*> {
	typedef TTrue TValue;
};

template <class T, class P>
struct TConDestructorVal {
	static void Construct(T *pT)                 { COMPILE_ASSERT(0); }
	static void ConstructCopy(T *pT, T const &t) { COMPILE_ASSERT(0); }
	static void Destroy(T *pT)                   { COMPILE_ASSERT(0); }
	static void Construct(T *pT, UINT uiCount)   { COMPILE_ASSERT(0); }
	static void Destroy(T *pT, UINT uiCount)     { COMPILE_ASSERT(0); }
};

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

// RTTI
class CObject;

class CRTTI {
public:
	static const int MAX_CLASSES = 512;

  typedef CObject *(*FObjectCreate)();

public:
	static int s_iClasses;
	static bool s_bSorted;
	static const CRTTI *s_pRTTIs[MAX_CLASSES];

	// Predicates for sorting / searching the RTTI array
	static inline bool Lt(const CRTTI *pRTTI1, const CRTTI *pRTTI2);
	static inline bool Lt(const char *pClassName, const CRTTI *pRTTI2);
	static inline bool Lt(const CRTTI *pRTTI1, const char *pClassName);

	static void Add(const CRTTI *pRTTI);
	static const CRTTI *Find(const char *pClassName);

public:
	char          *m_pClassName;
	const CRTTI   *m_pParent;
	FObjectCreate  m_fnCreate;
  QWORD          m_uiClassData;

	CRTTI(char *pClassName, FObjectCreate fnCreate, const CRTTI *pParent);
  inline CObject *CreateInstance() const;

	inline bool IsEqual(const CRTTI *pRTTI) const { return this == pRTTI; }
	inline bool IsKindOf(const CRTTI *pRTTI) const;
};


class CObject {
public:
	static CRTTI s_RTTI;
	static CObject *CreateInstance() { return new CObject(); }
	virtual const CRTTI *GetRTTI() const { return &s_RTTI; }
};

template <class T>
inline T *Cast(CObject *pObj)
{
	if (pObj && pObj->GetRTTI()->IsKindOf(&T::s_RTTI))
		return static_cast<T *>(pObj);
  return 0;
}

template <class T>
inline const T *Cast(const CObject *pObj)
{
	if (pObj && pObj->GetRTTI()->IsKindOf(&T::s_RTTI))
		return static_cast<const T *>(pObj);
  return 0;
}

template <class T>
inline bool IsKindOf(const CObject *pObj)
{
  return pObj && pObj->GetRTTI()->IsKindOf(&T::s_RTTI);
}

#define DEFRTTI_NOCREATE                                                \
	public:                                                               \
    static CRTTI s_RTTI;                                                \
		virtual const CRTTI *GetRTTI() const { return &s_RTTI; }

#define DEFRTTI                                                         \
  DEFRTTI_NOCREATE                                                      \
  static CObject *CreateInstance();

#define IMP_RTTI(Class, FCreate, RTTI)                                  \
    CRTTI Class::s_RTTI(#Class, FCreate, RTTI);

#define IMP_CREATE(Class)                                               \
  CObject *Class::CreateInstance() { return new Class(); }

#define IMPRTTI(Class, Base)                                            \
  IMP_CREATE(Class)                                                     \
  IMP_RTTI(Class, Class::CreateInstance, &Base::s_RTTI)

#define IMPRTTI_NOCREATE(Class, Base)                                   \
  IMP_RTTI(Class, 0, &Base::s_RTTI)

// Macro for template instantiations

#define IMPRTTI_T(Class, Base)                                          \
	template <>                                                           \
  IMP_CREATE(Class)                                                     \
	template <>                                                           \
  IMP_RTTI(Class, Class::CreateInstance, &Base::s_RTTI)

#define IMPRTTI_NOCREATE_T(Class, Base)                                 \
  template <>                                                           \
  IMP_RTTI(Class, 0, &Base::s_RTTI)

// RTTI Implementation --------------------------------------------------------

inline CObject *CRTTI::CreateInstance() const
{
	ASSERT(m_fnCreate);
	if (m_fnCreate)
		return m_fnCreate();
	return 0;
}

inline bool CRTTI::IsKindOf(const CRTTI *pRTTI) const
{
  const CRTTI *pRTTICur;
	for (pRTTICur = this; pRTTICur; pRTTICur = pRTTICur->m_pParent)
		if (pRTTI == pRTTICur)
			return true;
  return false;
}


#endif
