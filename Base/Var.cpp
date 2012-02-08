#include "stdafx.h"
#include "Var.h"
#include "VarUtil.h"
#include <stdio.h>

// Var classes RTTI -----------------------------------------------------

IMPRTTI_NOCREATE(CBaseVar, CObject)
IMPRTTI(CDummyVar, CBaseVar)

IMP_VAR_RTTI(int)
IMP_VAR_RTTI(float)
IMP_VAR_RTTI(CStr)
IMP_VAR_RTTI(BYTE)

IMPRTTI_NOCREATE_T(CVarValueBase<CVarObj>, CBaseVar) 
IMPRTTI_T(CVarRef<CVarObj>, CVarValueBase<CVarObj>)

// CVarObj --------------------------------------------------------------
IMPRTTI_NOCREATE(CVarObj, CObject)
IMPRTTI_NOCREATE(CVarObj::CIter, CObject)

bool CVarObj::GetStr(CStrBase const &sVar, CStrBase &s) const
{
  bool bRes;
  CVar<CStr> vStr;
  bRes = GetVar(sVar, vStr);
  s = vStr.Val();
  return bRes;
}

bool CVarObj::SetStr(CStrBase const &sVar, const CStrBase &s)
{
  bool bRes;
  CVar<CStr> vStr;
  vStr.Val() = s;
  bRes = SetVar(sVar, vStr);
  return bRes;
}

bool CVarObj::GetInt(CStrBase const &sVar, int &i) const
{
  bool bRes;
  CVar<int> vInt;
  bRes = GetVar(sVar, vInt);
  i = vInt.Val();
  return bRes;
}

bool CVarObj::SetInt(CStrBase const &sVar, int i)
{
  bool bRes;
  CVar<int> vInt;
  vInt.Val() = i;
  bRes = SetVar(sVar, vInt);
  return bRes;
}

bool CVarObj::GetFloat(CStrBase const &sVar, float &f) const
{
  bool bRes;
  CVar<float> vFloat;
  bRes = GetVar(sVar, vFloat);
  f = vFloat.Val();
  return bRes;
}

bool CVarObj::SetFloat(CStrBase const &sVar, float f)
{
  bool bRes;
  CVar<float> vFloat;
  vFloat.Val() = f;
  bRes = SetVar(sVar, vFloat);
  return bRes;
}

CVarObj *CVarObj::GetContext(CStrBase const &sVar)
{
  bool bRes;
  CVarRef<CVarObj> vContext;
  bRes = GetVar(sVar, vContext);
  if (bRes)
    return &vContext.Val();
  return 0;
}

bool CVarObj::GetVar(CStrBase const &sVar, CBaseVar &vDst) const
{
  CIter *pIt = GetIter(sVar);
  if (!pIt)
    return false;
  bool bRes = pIt->GetVar(vDst);
  delete pIt;
  return bRes;
}

bool CVarObj::SetVar(CStrBase const &sVar, const CBaseVar &vSrc)
{
  CIter *pIt = GetIter(sVar);
  if (!pIt)
    return false;
  bool bRes = pIt->SetVar(vSrc);
  delete pIt;
  return bRes;
}

// CVarValueObj ---------------------------------------------------------------
IMPRTTI_NOCREATE(CVarValueObj, CVarObj)

bool CVarValueObj::GetStr(CStrBase const &sVar, CStrBase &s) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  pVar->GetStr(s);
  return true;
}

bool CVarValueObj::SetStr(CStrBase const &sVar, const CStrBase &s)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = new CVar<CStr>();
    ReplaceVar(sVar, pVar, true);
  }
  pVar->SetStr(s);
  return true;
}

bool CVarValueObj::GetInt(CStrBase const &sVar, int &i) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  pVar->GetInt(i);
  return true;
}

bool CVarValueObj::SetInt(CStrBase const &sVar, int i)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = new CVar<int>();
    ReplaceVar(sVar, pVar, true);
  }
  pVar->SetInt(i);
  return true;
}

bool CVarValueObj::GetFloat(CStrBase const &sVar, float &f) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  pVar->GetFloat(f);
  return true;
}

bool CVarValueObj::SetFloat(CStrBase const &sVar, float f)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = new CVar<float>();
    ReplaceVar(sVar, pVar, true);
  }
  pVar->SetFloat(f);
  return true;
}

CVarObj *CVarValueObj::GetContext(CStrBase const &sVar)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar || !pVar->ValueHasRTTI())
    return 0;
  return Cast<CVarObj>((CObject *) pVar->GetRef());
}

bool CVarValueObj::GetVar(CStrBase const &sVar, CBaseVar &vDst) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  vDst.SetVar(*pVar);
  return true;
}

bool CVarValueObj::SetVar(CStrBase const &sVar, const CBaseVar &vSrc)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = vSrc.Clone();
    ReplaceVar(sVar, pVar, true);
    return true;
  }
  pVar->SetVar(vSrc);
  return true;
}

// Type conversion ------------------------------------------------------

bool Set(CStrBase *dst, const int *src)
{
  char chBuf[96];
  _itoa(*src, chBuf, 10);
  dst->Assign(CStrPart(chBuf));
  return true;
}

bool Set(int *dst, const CStrBase *src)
{
  if (src->ZeroTerminated())
    *dst = atoi(*src);
  else {
    CStr s = *src;
    *dst = atoi(s);
  }
  return true;
}

bool Set(CStrBase *dst, const float *src)
{
  char chBuf[96];
  sprintf(chBuf, "%g", *src);
  dst->Assign(CStrPart(chBuf));
  return true;
}

bool Set(float *dst, const CStrBase *src)
{
  int iRes;
  if (src->ZeroTerminated())
    iRes = sscanf(*src, "%f", dst);
  else {
    CStr s = *src;
    iRes = sscanf(s, "%f", dst);
  }
  if (iRes < 1) {
    *dst = 0;
    return false;
  }
  return true;
}

// CVarHash ----------------------------------------------------------------------

IMPRTTI(CVarHash, CVarObj)

CVarHash::CVarHash()
{
}

CVarHash::~CVarHash()
{
  m_Vars.DeleteAll();
}

CVarObj::CIter *CVarHash::GetIter(const CStrBase &sVar) const
{
  THash::TIter it;
  if (!!sVar) {
    if (sVar == GetLastIterConst()) 
      it.Init(&m_Vars, -1);
    else {
      it = m_Vars.Find(sVar);
      if (!it)
        return 0;
    }
  } else
    it = *(THash *) &m_Vars;
  return new CIter(it);
}

CBaseVar *CVarHash::FindVar(const CStrBase &sVar) const
{
  THash::TIter it;
  it = m_Vars.Find(sVar);
  if (!it)
    return 0;
  return it->pVar;
}

bool CVarHash::ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding)
{
  TVarName *pVarName;
  THash::TIter it;

  if (!pSrc) { // removing the var
    it = m_Vars.Find(sVar);
    if (!!it) {
      delete *it;
      m_Vars.Remove(it);
    }
    return true;
  }

  if (!bAdding) {
    it = m_Vars.Find(sVar);
    bAdding = !it;
  }
  if (bAdding) {
    pVarName = new TVarName(sVar, pSrc);
    m_Vars.Add(pVarName);
  } else 
    it->pVar = pSrc;

  return true;
}
