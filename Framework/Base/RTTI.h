#ifndef __RTTI_H
#define __RTTI_H

#include "Mem.h"
#include <string.h>

// RTTI

class CRTTI;
class CObject {
public:
  virtual ~CObject() {}

  typedef CRTTI TRTTI;
  static CObject      *CreateInstance_s(TAllocator &kAllocator) { return NEW_A(kAllocator, CObject, ()); }
  static char const    *GetClassName_s()   { return "CObject"; }
  static inline CRTTI const  *GetRTTI_s();
  virtual CRTTI const *GetRTTI() const    { return GetRTTI_s(); }
};

class CRTTI {
public:
  typedef CObject Type;

  static  CObject *CreateInstance_s(TAllocator &kAllocator)                 { return NEW_A(kAllocator, Type, ()); }
  virtual CObject *CreateInstance(TAllocator &kAllocator = DEF_ALLOC) const { return CreateInstance_s(kAllocator); }

  static  char const *GetName_s()                 { return Type::GetClassName_s(); }
  virtual char const *GetName() const             { return GetName_s(); }

  static  CRTTI const *GetBase_s()                { return 0; }
  virtual CRTTI const *GetBase() const            { return GetBase_s(); }

  static bool  IsEqual_s(CRTTI const *pRTTI)      { return pRTTI == Type::GetRTTI_s(); }
  virtual bool IsEqual(CRTTI const *pRTTI) const  { return IsEqual_s(pRTTI); }

  static bool  IsKindOf_s(CRTTI const *pRTTI)     { return IsEqual_s(pRTTI); }
  virtual bool IsKindOf(CRTTI const *pRTTI) const { return IsKindOf_s(pRTTI); }
};

template <class T, class B, bool C>
class CRTTITpl: public CRTTI {
public:
  typedef T Type;
  typedef B Base;

  static  CObject *CreateInstance_s(TAllocator &kAllocator)                 { ASSERT(!"Trying to instantiate class whose RTTI has creation turned off!"); return 0; }
  virtual CObject *CreateInstance(TAllocator &kAllocator = DEF_ALLOC) const { return CreateInstance_s(kAllocator); }

  static  char const *GetName_s()                 { return Type::GetClassName_s(); }
  virtual char const *GetName() const             { return GetName_s(); }

  static  CRTTI const *GetBase_s()                { return Base::GetRTTI_s(); }
  virtual CRTTI const *GetBase() const            { return GetBase_s(); }

  static bool  IsEqual_s(CRTTI const *pRTTI)      { return pRTTI == Type::GetRTTI_s(); }
  virtual bool IsEqual(CRTTI const *pRTTI) const  { return IsEqual_s(pRTTI); }

  static bool  IsKindOf_s(CRTTI const *pRTTI)     { return IsEqual_s(pRTTI) || Base::GetRTTI_s()->IsKindOf(pRTTI); }
  virtual bool IsKindOf(CRTTI const *pRTTI) const { return IsKindOf_s(pRTTI); }
};

template <class T, class B>
class CRTTITpl<T, B, true>: public CRTTITpl<T, B, false> {
public:
  static  CObject *CreateInstance_s(TAllocator &kAllocator)                 { return NEW_A(kAllocator, ID_TYPE(typename CRTTITpl<T, B, true>::Type), ()); }
  virtual CObject *CreateInstance(TAllocator &kAllocator = DEF_ALLOC) const { return CreateInstance_s(kAllocator); }
};

inline CRTTI const *CObject::GetRTTI_s()     { static TRTTI kRTTI; return &kRTTI; }

template <class T>
inline T *Cast(CObject *pObj)
{
	if (pObj && pObj->GetRTTI()->IsKindOf(T::GetRTTI_s()))
		return static_cast<T *>(pObj);
  return 0;
}

template <class T>
inline const T *Cast(const CObject *pObj)
{
	if (pObj && pObj->GetRTTI()->IsKindOf(T::GetRTTI_s()))
		return static_cast<const T *>(pObj);
  return 0;
}

template <class T>
inline bool IsKindOf(const CObject *pObj)
{
  return pObj && pObj->GetRTTI()->IsKindOf(T::GetRTTI_s());
}


class CRTTIHolder {
public:
	static const int MAX_CLASSES = 512;

public:
	int m_iClasses;
	bool m_bSorted;
	CRTTI const *m_pRTTIs[MAX_CLASSES];

	CRTTIHolder(): m_iClasses(0), m_bSorted(true) {}
	void Add(CRTTI const *pRTTI);
	CRTTI const *Find(char const *pClassName);

  static CRTTIHolder *Get() { static CRTTIHolder kHolder; return &kHolder; }

	// Predicates for sorting / searching the RTTI array
	static inline bool Lt(CRTTI const *pRTTI1, CRTTI const *pRTTI2)    { return strcmp(pRTTI1->GetName(), pRTTI2->GetName()) < 0; }
	static inline bool Lt(char const *pClassName, CRTTI const *pRTTI2) { return strcmp(pClassName, pRTTI2->GetName()) < 0; }
	static inline bool Lt(CRTTI const *pRTTI1, char const *pClassName) { return strcmp(pRTTI1->GetName(), pClassName) < 0; }
};

template <class T>
class CRTTIRegisterer {
public:
  CRTTIRegisterer() { CRTTIHolder::Get()->Add(T::GetRTTI_s()); }
};

#define DEFRTTI(Class, Base, Create) \
  public: \
    typedef CRTTITpl<Class, Base, Create> TRTTI; \
    static char const   *GetClassName_s() { return #Class; } \
    static CRTTI const  *GetRTTI_s()      { static TRTTI kRTTI; return &kRTTI; } \
    virtual CRTTI const *GetRTTI() const  { return GetRTTI_s(); }

#endif
