#ifndef __VAR_H
#define __VAR_H

#include "Str.h"

// Var values -------------------------------------------------------------------------

class CBaseVar: public CObject {
	DEFRTTI_NOCREATE
  DEFREFCOUNT_DUMMY
public:
  static const unsigned int REF_INVALID = (unsigned int) -1;

  virtual ~CBaseVar() {}

  virtual void *GetPtr()  const = 0;
  virtual int   GetSize() const = 0;

  virtual bool GetStr(CStrBase &s) const    = 0;
  virtual bool SetStr(CStrBase const &s)    = 0;

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
class CDummyVar: public CBaseVar {
	DEFRTTI
public:
  virtual void *GetPtr()  const             { return 0;                          }
  virtual int   GetSize() const             { return 0;                          }

  virtual bool GetStr(CStrBase &s) const    { s.Assign(CStrPart()); return true; }
  virtual bool SetStr(CStrBase const &s)    { return true;                       }

  virtual bool GetInt(int &i) const         { i = 0; return true;                }
  virtual bool SetInt(int i)                { return true;                       }

  virtual bool GetFloat(float &f) const     { f = 0.0f; return true;             }
  virtual bool SetFloat(float f)            { return true;                       }

  virtual bool SetVar(const CBaseVar &vSrc) { return true;                       }

  virtual void *GetRef() const              { return (void *) REF_INVALID;       }
  virtual bool  SetRef(void *pPtr)          { return false;                      }

  virtual bool ValueHasRTTI() const         { return false;                      }

  virtual CBaseVar *Clone() const           { return new CDummyVar();            }
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

template <class T>
class CVarValueBase: public CBaseVar {
  DEFRTTI_NOCREATE
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

  bool GetStr(CStrBase &s) const { return Set(&s, &Val()); }
  bool SetStr(CStrBase const &s) { return Set(&Val(), &s); }

  bool GetInt(int &i) const      { return Set(&i, &Val()); }
  bool SetInt(int i)             { return Set(&Val(), &i); }

  bool GetFloat(float &f) const  { return Set(&f, &Val()); }
  bool SetFloat(float f)         { return Set(&Val(), &f); }

  bool SetVar(CBaseVar const &vSrc) { return SetValue(&Val(), &vSrc); }

  void *GetRef() const     { return m_Val.GetRef();     }
  bool  SetRef(void *pPtr) { return m_Val.SetRef(pPtr); }

  CBaseVar *Clone() const { CVarTpl<V> *pVar = new CVarTpl<V>(); Set(&pVar->Val(), &Val()); return pVar; }
};

// Variable with value and variable with reference to a value

template <class T>
class CVar: public CVarTpl<CVal<T> > {
	DEFRTTI
public:
  CVar(): CVarTpl()            {}
  CVar(T const &t): CVarTpl(t) {}
};

template <class T>
class CVarRef: public CVarTpl<CRef<T> > {
	DEFRTTI
public:
  CVarRef(): CVarTpl()      {}
  CVarRef(T &t): CVarTpl(t) {}

  CBaseVar *Clone() const { CVarRef<T> *pVar = new CVarRef<T>(*(T*) GetRef()); return pVar; }
};

// Var Objects - collections of named values ----------------------------------

class CVarObj: public CObject {
	DEFRTTI_NOCREATE
public:
	class CIter: public CObject {
    DEFRTTI_NOCREATE
	public:
		virtual ~CIter() {}

    virtual CIter &Next()                     = 0;
    virtual CIter &Prev()                     = 0;
    virtual operator bool () const            = 0;
                                              
		virtual const CStrConst GetName()         = 0;
		virtual bool GetVar(CBaseVar &vDst) const = 0;
		virtual bool SetVar(CBaseVar const &vSrc) = 0;

    virtual CBaseVar *GetValue()              = 0;
		virtual bool SetValue(CBaseVar *pVar)     = 0;
	};

  virtual ~CVarObj() {}

  virtual bool GetStr(CStrBase const &sVar, CStrBase &s) const;
  virtual bool SetStr(CStrBase const &sVar, const CStrBase &s);

  virtual bool GetInt(CStrBase const &sVar, int &i) const;
  virtual bool SetInt(CStrBase const &sVar, int i);

  virtual bool GetFloat(CStrBase const &sVar, float &f) const;
  virtual bool SetFloat(CStrBase const &sVar, float f);

  virtual CVarObj *GetContext(const CStrBase &sVar);

  // Empty string gets iterator to the first element, "@last" gets iterator to the last element
  virtual CIter *GetIter(const CStrBase &sVar = CStrPart()) const = 0;

	virtual CBaseVar *FindVar(const CStrBase &sVar) const                               = 0;
  virtual bool ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding = false) = 0;

  virtual bool GetVar(const CStrBase &sVar, CBaseVar &vDst) const;
  virtual bool SetVar(const CStrBase &sVar, const CBaseVar &vSrc);

  static const CStrConst GetLastIterConst() { static const CStrConst sLast("@last"); return sLast; }
};

// Collection of CVar objects -------------------------------------------------

class CVarValueObj: public CVarObj {
  DEFRTTI_NOCREATE
public:
  virtual ~CVarValueObj() {}

  virtual bool GetStr(CStrBase const &sVar, CStrBase &s) const;
  virtual bool SetStr(CStrBase const &sVar, const CStrBase &s);

  virtual bool GetInt(CStrBase const &sVar, int &i) const;
  virtual bool SetInt(CStrBase const &sVar, int i);

  virtual bool GetFloat(CStrBase const &sVar, float &f) const;
  virtual bool SetFloat(CStrBase const &sVar, float f);

  virtual CVarObj *GetContext(const CStrBase &sVar);

  virtual bool GetVar(const CStrBase &sVar, CBaseVar &vDst) const;
  virtual bool SetVar(const CStrBase &sVar, const CBaseVar &vSrc);
};

class CVarHash: public CVarValueObj {
	DEFRTTI
public:
  struct TVarName {
    CStrConst           sName;
    CSmartPtr<CBaseVar> pVar;

    TVarName(const CStrBase &sN, CBaseVar *pV): sName(sN), pVar(pV) {}
    ~TVarName() {}

    static inline bool Eq(const CStrBase &s, TVarName *pVarName) { return pVarName->sName == s;      }
    static inline size_t Hash(const CStrBase &s)                 { return s.GetHash();               }
    static inline size_t Hash(TVarName *pVarName)                { return pVarName->sName.GetHash(); }
  };

  typedef CHash<TVarName *, const CStrBase &, TVarName, TVarName> THash;

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

		virtual const CStrConst GetName()         { return m_it->sName; }
    virtual bool GetVar(CBaseVar &vDst) const { return vDst.SetVar(*m_it->pVar); }
    virtual bool SetVar(CBaseVar const &vSrc) { return m_it->pVar->SetVar(vSrc); }

		virtual CBaseVar *GetValue()              { return m_it->pVar; }
		virtual bool SetValue(CBaseVar *pVar)     { m_it->pVar = pVar; return true; }
	};

public:
  CVarHash();
  virtual ~CVarHash();

	virtual CVarObj::CIter *GetIter(const CStrBase &sVar = CStrPart()) const;
	virtual CBaseVar *FindVar(const CStrBase &sVar) const;
  virtual bool ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding = false);
};

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

inline bool Set(BYTE *dst, const int *src)
{
  *dst = (BYTE) *src;
  return *dst == *src;
}

inline bool Set(int *dst, const BYTE *src)
{
  *dst = (int) *src;
  return true;
}

inline bool Set(BYTE *dst, const float *src)
{
  *dst = (BYTE) *src;
  return *dst == *src;
}

inline bool Set(float *dst, const BYTE *src)
{
  *dst = (float) *src;
  return true;
}

bool Set(CStrBase *dst, const int *src);
bool Set(int *dst, const CStrBase *src);
bool Set(CStrBase *dst, const float *src);
bool Set(float *dst, const CStrBase *src);
inline bool Set(CStrBase *dst, const BYTE *src) { int i = *src; return Set(dst, &i); }
inline bool Set(BYTE *dst, const CStrBase *src) { int i; bool bRes = Set(&i, src); *dst = (BYTE) i; return bRes && *dst == i; }

inline bool Set(CStrBase *dst, const CStrBase *src)
{
  dst->Assign(*src);
  ASSERT(*dst == *src);
  return true;
}

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

static inline bool SetValue(CStr *val, const CBaseVar *vSrc)
{
  return vSrc->GetStr(*val);
}

static inline bool SetValue(BYTE *val, const CBaseVar *vSrc)
{
  int i;
  bool bRes = vSrc->GetInt(i); 
  *val = (BYTE) i;
  return bRes && *val == i;
}

// HasRTTI implementation ---------------------------------------------------------------------

inline bool HasRTTI(const void *)    { return false; }
inline bool HasRTTI(const CObject *) { return true;  }

// Macros for implementing trivial Var set methods, i.e. no conversion to basic types and setting from its own type value or Var
#define IMPLEMENT_NO_BASIC_SET(TYPE) \
  inline bool Set(TYPE *dst, int const *src) { ASSERT(0); return false; } \
  inline bool Set(TYPE *dst, float const *src) { ASSERT(0); return false; } \
  inline bool Set(TYPE *dst, CStrBase const *src) { ASSERT(0); return false; } \
  inline bool Set(int *dst,  TYPE const *src) { ASSERT(0); return false; } \
  inline bool Set(float *dst, TYPE const *src) { ASSERT(0); return false; } \
  inline bool Set(CStrBase *dst, TYPE const *src) { ASSERT(0); return false; }

#define IMPLEMENT_NO_SET(TYPE) \
  IMPLEMENT_NO_BASIC_SET(TYPE) \
  inline bool Set(TYPE *dst, TYPE const *src) { ASSERT(0); return false; } \
  inline bool SetValue(TYPE *val, CBaseVar const *vSrc) { ASSERT(0); return false; }

#define IMPLEMENT_BASE_SET(TYPE) \
  IMPLEMENT_NO_BASIC_SET(TYPE) \
  inline bool Set(TYPE *dst, TYPE const *src) { *dst = (TYPE) *src; return true; } \
  inline bool SetValue(TYPE *val, CBaseVar const *vSrc) { return SetValueBase(val, vSrc); }

// Var RTTI ---------------------------------------------------------

#define IMP_VAR_RTTI(Type)                          \
  IMPRTTI_NOCREATE_T(CVarValueBase<Type>, CBaseVar) \
  IMPRTTI_T(CVar<Type>, CVarValueBase<Type>)        \
  IMPRTTI_T(CVarRef<Type>, CVarValueBase<Type>)


#endif
