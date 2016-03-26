#ifndef __VAR_H
#define __VAR_H

#include "RTTI.h"
#include "Str.h"

// Var values -------------------------------------------------------------------------

class CBaseVar: public CObject {
  DEFRTTI(CBaseVar, CObject, false)
  DEFREFCOUNT_DUMMY
public:
  static const unsigned int REF_INVALID = (unsigned int) -1;

  virtual ~CBaseVar() {}

  virtual void *GetPtr()  const = 0;
  virtual int   GetSize() const = 0;

  virtual bool GetStr(CStrAny &s) const     = 0;
  virtual bool SetStr(CStrAny const &s)     = 0;

  virtual bool GetInt(int &i) const         = 0;
  virtual bool SetInt(int i)                = 0;

  virtual bool GetFloat(float &f) const     = 0;
  virtual bool SetFloat(float f)            = 0;

  virtual bool SetVar(CBaseVar const &vSrc) = 0;

  virtual void *GetRef() const              = 0;
  virtual bool  SetRef(void *pPtr)          = 0;

  virtual bool ValueHasRTTI() const         = 0;

  virtual CBaseVar *Clone() const           = 0;
};

// Dummy var - just a method sink
class CDummyVar;
template <>
struct TSpecifyAllocator<CDummyVar> { typedef TGetAlloc<CBaseVar>::Type Type; };

class CDummyVar: public CBaseVar {
  DEFRTTI(CDummyVar, CBaseVar, true)
public:
  virtual void *GetPtr()  const             { return 0;                          }
  virtual int   GetSize() const             { return 0;                          }

  virtual bool GetStr(CStrAny &s) const     { s.Clear(); return true;            }
  virtual bool SetStr(CStrAny const &s)     { return true;                       }

  virtual bool GetInt(int &i) const         { i = 0; return true;                }
  virtual bool SetInt(int i)                { return true;                       }

  virtual bool GetFloat(float &f) const     { f = 0.0f; return true;             }
  virtual bool SetFloat(float f)            { return true;                       }

  virtual bool SetVar(const CBaseVar &vSrc) { return true;                       }

  virtual void *GetRef() const              { return (void *) REF_INVALID;       }
  virtual bool  SetRef(void *pPtr)          { return false;                      }

  virtual bool ValueHasRTTI() const         { return false;                      }

  virtual CBaseVar *Clone() const           { return NEW(CDummyVar, ());         }
};

// Value wrappers
template <class T>
class CVal {
public:
  typedef T TYPE;

  T m_Val;

  inline CVal()                     {}
  inline CVal(const T &t): m_Val(t) {}

  inline       T &Val()       { return m_Val; }
  inline const T &Val() const { return m_Val; }

  inline void *GetRef() const     { return (void *) &m_Val; }
  inline bool  SetRef(void *pPtr) { return false;           }
};

template <class T>
class CRef {
public:
  typedef T TYPE;

  T *m_pVal;

  inline CRef()                 {}
  inline CRef(T &t): m_pVal(&t) {}

  inline       T &Val()       { return *m_pVal; }
  inline const T &Val() const { return *m_pVal; }

  inline void *GetRef() const     { return (void *) m_pVal;           }
  inline bool  SetRef(void *pPtr) { m_pVal = (T *) pPtr; return true; }
};

// Variable template

// Data Conversion implementation - copying of values -----------------------------------------

template <class T>
static inline bool Set(T *dst, const T *src)
{
  *dst = *src;
  return true;
}

inline bool Set(float *dst, const int *src)
{
  *dst = (float) *src;
  return *dst == *src;
}

inline bool Set(int *dst, const float *src)
{
  *dst = (int) *src;
  return *dst == *src;
}

inline bool Set(uint8_t *dst, const int *src)
{
  *dst = (uint8_t) *src;
  return *dst == *src;
}

inline bool Set(int *dst, const uint8_t *src)
{
  *dst = (int) *src;
  return true;
}

inline bool Set(uint8_t *dst, const float *src)
{
  *dst = (uint8_t) *src;
  return *dst == *src;
}

inline bool Set(float *dst, const uint8_t *src)
{
  *dst = (float) *src;
  return true;
}

bool Set(CStrAny *dst, const int *src);
bool Set(int *dst, const CStrAny *src);
bool Set(CStrAny *dst, const float *src);
bool Set(float *dst, const CStrAny *src);
inline bool Set(CStrAny *dst, const uint8_t *src) { int i = *src; return Set(dst, &i); }
inline bool Set(uint8_t *dst, const CStrAny *src) { int i; bool bRes = Set(&i, src); *dst = (uint8_t) i; return bRes && *dst == i; }

inline bool Set(CStrAny *dst, const CStrAny *src)
{
  *dst = *src;
  dst->AssureHasHeader();
  ASSERT(*dst == *src);
  return true;
}

// HasRTTI implementation ---------------------------------------------------------------------

inline bool HasRTTI(void const *)    { return false; }
inline bool HasRTTI(CObject const *) { return true;  }


template <class T>
class CVarValueBase: public CBaseVar {
  DEFRTTI(CVarValueBase<T>, CBaseVar, false)
public:
  virtual       T& GetValue()       = 0;
  virtual const T& GetValue() const = 0;
};

template <class V>
class CVarTpl: public CVarValueBase<typename V::TYPE> {
public:
  typedef typename V::TYPE TYPE;

  V m_Val;

  CVarTpl()                        {}
  CVarTpl(      TYPE &t): m_Val(t) {}
  CVarTpl(const TYPE &t): m_Val(t) {}

  inline       TYPE &Val()       { return m_Val.Val(); }
  inline const TYPE &Val() const { return m_Val.Val(); }

  TYPE& GetValue()             { return Val(); }
  const TYPE& GetValue() const { return Val(); }

  bool ValueHasRTTI() const { return HasRTTI(&Val()); }

  void *GetPtr()  const { return (void *) &Val(); }
  int   GetSize() const { return sizeof(Val());   }

  bool GetStr(CStrAny &s) const  { return Set(&s, &Val()); }
  bool SetStr(CStrAny const &s)  { return Set(&Val(), &s); }

  bool GetInt(int &i) const      { return Set(&i, &Val()); }
  bool SetInt(int i)             { return Set(&Val(), &i); }

  bool GetFloat(float &f) const  { return Set(&f, &Val()); }
  bool SetFloat(float f)         { return Set(&Val(), &f); }

  bool SetVar(CBaseVar const &vSrc) { return SetValue(&Val(), &vSrc); }

  void *GetRef() const     { return m_Val.GetRef();     }
  bool  SetRef(void *pPtr) { return m_Val.SetRef(pPtr); }

  CBaseVar *Clone() const { CVarTpl<V> *pVar = NEW(CVarTpl<V>, ()); Set(&pVar->Val(), &Val()); return pVar; }
};

// Variable with value and variable with reference to a value

template <class T>
class CVar: public CVarTpl<CVal<T> > {
	DEFRTTI(CVar<T>, CVarTpl<CVal<T> >, true)
public:
  CVar(): CVarTpl<CVal<T> >()            {}
  CVar(T const &t): CVarTpl<CVal<T> > (t) {}
};

template <class T>
struct TSpecifyAllocator<CVar<T> > { typedef TGetAlloc<CBaseVar>::Type Type; };

template <class T>
class CVarRef: public CVarTpl<CRef<T> > {
	DEFRTTI(CVarRef<T>, CVarTpl<CRef<T> >, true)
public:
  using CVarTpl<CRef<T> >::GetRef;

  CVarRef(): CVarTpl<CRef<T> >()      {}
  CVarRef(T &t): CVarTpl<CRef<T> >(t) {}

  CBaseVar *Clone() const { CVarRef<T> *pVar = NEW(CVarRef<T>, (*(T*) GetRef())); return pVar; }
};

template <class T>
struct TSpecifyAllocator<CVarRef<T> > { typedef TGetAlloc<CBaseVar>::Type Type; };

// Var Objects - collections of named values ----------------------------------

class CVarObj: public CObject {
	DEFRTTI(CVarObj, CObject, false)
public:
	class CIter: public CObject {
    DEFRTTI(CVarObj::CIter, CObject, false)
	public:
		virtual ~CIter() {}

    virtual CIter &Next()                     = 0;
    virtual CIter &Prev()                     = 0;
    virtual operator bool () const            = 0;

		virtual CStrAny GetName() const           = 0;
		virtual bool GetVar(CBaseVar &vDst) const = 0;
		virtual bool SetVar(CBaseVar const &vSrc) = 0;

    virtual CBaseVar *GetValue()              = 0;
		virtual bool SetValue(CBaseVar *pVar)     = 0;
	};

  virtual ~CVarObj() {}

  virtual bool GetStr(CStrAny const &sVar, CStrAny &s) const;
  virtual bool SetStr(CStrAny const &sVar, const CStrAny &s);

  virtual bool GetInt(CStrAny const &sVar, int &i) const;
  virtual bool SetInt(CStrAny const &sVar, int i);

  virtual bool GetFloat(CStrAny const &sVar, float &f) const;
  virtual bool SetFloat(CStrAny const &sVar, float f);

  virtual CVarObj *GetContext(const CStrAny &sVar);

  // Empty string gets iterator to the first element, "@last" gets iterator to the last element
  virtual CIter *GetIter(const CStrAny &sVar = CStrAny(ST_PART)) const = 0;

	virtual CBaseVar *FindVar(const CStrAny &sVar) const                               = 0;
  virtual bool ReplaceVar(const CStrAny &sVar, CBaseVar *pSrc, bool bAdding = false) = 0;

  virtual bool GetVar(const CStrAny &sVar, CBaseVar &vDst) const;
  virtual bool SetVar(const CStrAny &sVar, const CBaseVar &vSrc);

  static const CStrAny &GetLastIterConst() { static const CStrAny sLast(ST_WHOLE, "@last"); return sLast; }
};

template <>
struct TSpecifyAllocator<CVarObj> { typedef TGetAlloc<CBaseVar>::Type Type; };

// Collection of CVar objects -------------------------------------------------

class CVarValueObj: public CVarObj {
  DEFRTTI(CVarValueObj, CVarObj, false)
public:
  virtual ~CVarValueObj() {}

  virtual bool GetStr(CStrAny const &sVar, CStrAny &s) const;
  virtual bool SetStr(CStrAny const &sVar, const CStrAny &s);

  virtual bool GetInt(CStrAny const &sVar, int &i) const;
  virtual bool SetInt(CStrAny const &sVar, int i);

  virtual bool GetFloat(CStrAny const &sVar, float &f) const;
  virtual bool SetFloat(CStrAny const &sVar, float f);

  virtual CVarObj *GetContext(const CStrAny &sVar);

  virtual bool GetVar(const CStrAny &sVar, CBaseVar &vDst) const;
  virtual bool SetVar(const CStrAny &sVar, const CBaseVar &vSrc);
};

template <>
struct TSpecifyAllocator<CVarValueObj> { typedef TGetAlloc<CVarObj>::Type Type; };

class CVarHash: public CVarValueObj {
  DEFRTTI(CVarHash, CVarValueObj, true)
public:
  struct TVarName {
    CSmartPtr<CStrHeader const> pName;
    CSmartPtr<CBaseVar>         pVar;

    TVarName(const CStrAny &sN, CBaseVar *pV): pName(sN.GetHeaderForContents(true)), pVar(pV) {}
    ~TVarName() {}

    static inline bool Eq(CStrAny const &s, TVarName *pVarName)          { return CStrAny::Eq(pVarName->pName, s);                    }
    static inline bool Eq(CStrHeader const *pHeader, TVarName *pVarName) { return CStrHeader::Eq(pHeader, pVarName->pName);           }
		static inline bool Eq(TVarName *pVarName0, TVarName *pVarName1)      { return CStrHeader::Eq(pVarName0->pName, pVarName1->pName); }
    static inline size_t Hash(const CStrAny &s)                          { return s.GetHash();                                        }
    static inline size_t Hash(CStrHeader const *pHeader)                 { return pHeader->GetHash();                                 }
    static inline size_t Hash(TVarName *pVarName)                        { return pVarName->pName->GetHash();                         }
  };

  typedef CHash<TVarName *, const CStrAny &, TVarName, TVarName> THash;

public:
  THash m_Vars;

public:
	class CIter: public CVarObj::CIter {
	public:
	  THash::TIter m_it;

    CIter()                          {}
    CIter(THash::TIter it): m_it(it) {}
    CIter(CVarObj &kObj)             { Set(&kObj); }

    void Set(CVarObj *pObj) { CVarHash *pHash = Cast<CVarHash>(pObj); ASSERT(pHash); if (pHash) m_it = pHash->m_Vars; }

    virtual CIter &operator =(CVarObj &kVarHash) { Set(&kVarHash); return *this; }

    virtual CIter &Next() { ++m_it; return *this; }
    virtual CIter &Prev() { --m_it; return *this; }
    virtual operator bool () const { return (bool) m_it; }

    virtual CStrAny GetName() const           { return CStrAny(m_it->pName); }
    virtual bool GetVar(CBaseVar &vDst) const { return vDst.SetVar(*m_it->pVar); }
    virtual bool SetVar(CBaseVar const &vSrc) { return m_it->pVar->SetVar(vSrc); }

		virtual CBaseVar *GetValue()              { return m_it->pVar; }
		virtual bool SetValue(CBaseVar *pVar)     { m_it->pVar = pVar; return true; }
	};

public:
  CVarHash();
  virtual ~CVarHash();

	virtual CVarObj::CIter *GetIter(const CStrAny &sVar = CStrAny(ST_PART)) const;
	virtual CBaseVar *FindVar(const CStrAny &sVar) const;
  virtual bool ReplaceVar(const CStrAny &sVar, CBaseVar *pSrc, bool bAdding = false);
};

template <>
struct TSpecifyAllocator<CVarHash::TVarName> { typedef TGetAlloc<CBaseVar>::Type Type; };

template <>
struct TSpecifyAllocator<CVarHash> { typedef TGetAlloc<CVarObj>::Type Type; };

// SetValue implementation - initialization of a value from a var ----------------------------

// Catch-all template that can assign a value from a var holding the same value type
template <class T>
static inline bool SetValueBase(T *val, const CBaseVar *vSrc)
{
  const CVarValueBase<T> *pV = Cast<CVarValueBase<T> >(vSrc);
  if (pV)
    return Set(val, &pV->GetValue());
  return false;
}

static inline bool SetValue(int *val, const CBaseVar *vSrc)
{
  return vSrc->GetInt(*val);
}

static inline bool SetValue(float *val, const CBaseVar *vSrc)
{
  return vSrc->GetFloat(*val);
}

static inline bool SetValue(CStrAny *val, const CBaseVar *vSrc)
{
  return vSrc->GetStr(*val);
}

static inline bool SetValue(uint8_t *val, const CBaseVar *vSrc)
{
  int i;
  bool bRes = vSrc->GetInt(i);
  *val = (uint8_t) i;
  return bRes && *val == i;
}

// Macros for implementing trivial Var set methods, i.e. no conversion to basic types and setting from its own type value or Var
#define IMPLEMENT_NO_BASIC_SET(TYPE) \
  inline bool Set(TYPE *dst, int const *src) { ASSERT(0); return false; } \
  inline bool Set(TYPE *dst, float const *src) { ASSERT(0); return false; } \
  inline bool Set(TYPE *dst, CStrAny const *src) { ASSERT(0); return false; } \
  inline bool Set(int *dst,  TYPE const *src) { ASSERT(0); return false; } \
  inline bool Set(float *dst, TYPE const *src) { ASSERT(0); return false; } \
  inline bool Set(CStrAny *dst, TYPE const *src) { ASSERT(0); return false; }

#define IMPLEMENT_NO_SET(TYPE) \
  IMPLEMENT_NO_BASIC_SET(TYPE) \
  inline bool Set(TYPE *dst, TYPE const *src) { ASSERT(0); return false; } \
  inline bool SetValue(TYPE *val, CBaseVar const *vSrc) { ASSERT(0); return false; }

#define IMPLEMENT_BASE_SET(TYPE) \
  IMPLEMENT_NO_BASIC_SET(TYPE) \
  inline bool Set(TYPE *dst, TYPE const *src) { *dst = (TYPE) *src; return true; } \
  inline bool SetValue(TYPE *val, CBaseVar const *vSrc) { return SetValueBase(val, vSrc); }

// Var RTTI ---------------------------------------------------------

template <class T>
class CVarRTTIRegisterer {
  CRTTIRegisterer<CVarValueBase<T> > m_RegVarValueBase;
  CRTTIRegisterer<CVar<T> > m_RegVar;
  CRTTIRegisterer<CVarRef<T> > m_RegVarRef;
};

#endif
