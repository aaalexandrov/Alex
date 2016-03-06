#ifndef __VARUTIL_H
#define __VARUTIL_H

#include "Var.h"

class CVarMerge: public CVarValueObj {
  DEFRTTI(CVarMerge, CVarValueObj, false)
public:
  class CIter: public CVarObj::CIter {
    DEFRTTI(CVarMerge::CIter, CVarObj::CIter, false)
  public:
    CVarMerge const *m_pMerge;
    int              m_iVar;
    CVarObj::CIter  *m_pIt;

    CIter(CVarMerge const *pMerge, int iVar, CVarObj::CIter *pIt, int iDirection = 1);
    virtual ~CIter() { DEL(m_pIt); }

    virtual CIter &Next();
    virtual CIter &Prev();
    virtual operator bool () const            { return m_pIt && *m_pIt;     }

    virtual CStrAny GetName() const           { return m_pIt->GetName();    }
    virtual bool GetVar(CBaseVar &vDst) const { return m_pIt->GetVar(vDst); }
    virtual bool SetVar(CBaseVar const &vSrc) { return m_pIt->SetVar(vSrc); }
    virtual CBaseVar *GetValue()              { return m_pIt->GetValue();     }
    virtual bool SetValue(CBaseVar *pVar)     { return m_pIt->SetValue(pVar); }

    bool IsValidPos();
  };

  struct TVarInfo {
    CVarObj *m_pVar;
    bool     m_bOwned;
  };
public:
  CArray<TVarInfo> m_Vars;

  CVarMerge() {}
  CVarMerge(CVarObj *pInherited, CVarObj *pBase, bool bOwnInherited, bool bOwnBase);
  virtual ~CVarMerge();

  virtual int AppendBaseVar(CVarObj *pVar, bool bOwn);
  virtual void RemoveLastBaseVar();
  virtual void ClearBaseVars();

	virtual CVarObj::CIter *GetIter(CStrAny const &sVar = CStrAny()) const;
	virtual CBaseVar *FindVar(CStrAny const &sVar) const;
  virtual bool ReplaceVar(CStrAny const &sVar, CBaseVar *pSrc, bool bAdding = false);
};

template <>
struct TGetAllocator<CVarMerge> { typedef TGetAllocator<CVarObj>::Type Type; };

class CVarTemplate: public CObject {
  DEFRTTI(CVarTemplate, CObject, true)
public:
  CVarObj *m_pVars;

  CVarTemplate()             { m_pVars = 0; }
  virtual ~CVarTemplate()    {}

  virtual bool Init()        { return true; }
  virtual void Done()        { if (m_pVars) { DEL(m_pVars); m_pVars = 0; } }

  virtual void Remap(uint8_t *pNewBufBase, uint8_t *pOldBufBase, CVarObj *pDstVars = 0);
  virtual void MakeVarCopy(CVarObj *pDstVars, uint8_t *pDstBufBase, uint8_t *pSrcBufBase, bool bAddVars);
  virtual void ClearVarCopy(CVarObj *pDstVars, uint8_t *pDstBufBase, int iDstBufSize);

  virtual void CopyValues(CVarObj *pSrcVars);
};

template <>
struct TGetAllocator<CVarTemplate> { typedef TGetAllocator<CVarObj>::Type Type; };

class CFile;

class CVarFile {
public:
  CFile *m_pFile;

  CVarFile(CFile *pFile);
  ~CVarFile();

  bool Read(CVarObj *pVars);
  bool Write(CVarObj *pVars, int iIndent = 0);

  bool ReadVarStrings(CStrAny &sName, CStrAny &sVal, const CRTTI *pVarRTTI);

  bool WriteIndent(int iIndent);
};

// Var contexts

inline bool Set(CVarObj *dst, int const *src) { ASSERT(0); return false; }
inline bool Set(CVarObj *dst, float const *src) { ASSERT(0); return false; }
inline bool Set(int *dst,  CVarObj const *src) { ASSERT(0); return false; }
inline bool Set(float *dst, CVarObj const *src) { ASSERT(0); return false; }

bool Set(CStrAny *dst, CVarObj const *src);
bool Set(CVarObj *dst, CStrAny const *src);
bool Set(CVarObj *dst, CVarObj const *src);
bool SetValue(CVarObj *val, CBaseVar const *vSrc);

#endif
