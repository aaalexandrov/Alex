#include "stdafx.h"
#include "Var.h"
#include "VarUtil.h"
#include <stdio.h>

// Var classes RTTI -----------------------------------------------------

IMPRTTI_NOCREATE(CBaseVar, CObject)
IMPRTTI(CDummyVar, CBaseVar)

IMP_VAR_RTTI(int)
IMP_VAR_RTTI(float)
IMP_VAR_RTTI(CStrAny)
IMP_VAR_RTTI(BYTE)

IMPRTTI_NOCREATE_T(CVarValueBase<CVarObj>, CBaseVar) 
IMPRTTI_T(CVarRef<CVarObj>, CVarValueBase<CVarObj>)

// CVarObj --------------------------------------------------------------
IMPRTTI_NOCREATE(CVarObj, CObject)
IMPRTTI_NOCREATE(CVarObj::CIter, CObject)

bool CVarObj::GetStr(CStrAny const &sVar, CStrAny &s) const
{
  bool bRes;
  CVar<CStrAny> vStr;
  bRes = GetVar(sVar, vStr);
  s = vStr.Val();
  return bRes;
}

bool CVarObj::SetStr(CStrAny const &sVar, const CStrAny &s)
{
  bool bRes;
  CVar<CStrAny> vStr;
  vStr.Val() = s;
  bRes = SetVar(sVar, vStr);
  return bRes;
}

bool CVarObj::GetInt(CStrAny const &sVar, int &i) const
{
  bool bRes;
  CVar<int> vInt;
  bRes = GetVar(sVar, vInt);
  i = vInt.Val();
  return bRes;
}

bool CVarObj::SetInt(CStrAny const &sVar, int i)
{
  bool bRes;
  CVar<int> vInt;
  vInt.Val() = i;
  bRes = SetVar(sVar, vInt);
  return bRes;
}

bool CVarObj::GetFloat(CStrAny const &sVar, float &f) const
{
  bool bRes;
  CVar<float> vFloat;
  bRes = GetVar(sVar, vFloat);
  f = vFloat.Val();
  return bRes;
}

bool CVarObj::SetFloat(CStrAny const &sVar, float f)
{
  bool bRes;
  CVar<float> vFloat;
  vFloat.Val() = f;
  bRes = SetVar(sVar, vFloat);
  return bRes;
}

CVarObj *CVarObj::GetContext(CStrAny const &sVar)
{
  bool bRes;
  CVarRef<CVarObj> vContext;
  bRes = GetVar(sVar, vContext);
  if (bRes)
    return &vContext.Val();
  return 0;
}

bool CVarObj::GetVar(CStrAny const &sVar, CBaseVar &vDst) const
{
  CIter *pIt = GetIter(sVar);
  if (!pIt)
    return false;
  bool bRes = pIt->GetVar(vDst);
  delete pIt;
  return bRes;
}

bool CVarObj::SetVar(CStrAny const &sVar, const CBaseVar &vSrc)
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

bool CVarValueObj::GetStr(CStrAny const &sVar, CStrAny &s) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  pVar->GetStr(s);
  return true;
}

bool CVarValueObj::SetStr(CStrAny const &sVar, const CStrAny &s)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = new CVar<CStrAny>();
    ReplaceVar(sVar, pVar, true);
  }
  pVar->SetStr(s);
  return true;
}

bool CVarValueObj::GetInt(CStrAny const &sVar, int &i) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  pVar->GetInt(i);
  return true;
}

bool CVarValueObj::SetInt(CStrAny const &sVar, int i)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = new CVar<int>();
    ReplaceVar(sVar, pVar, true);
  }
  pVar->SetInt(i);
  return true;
}

bool CVarValueObj::GetFloat(CStrAny const &sVar, float &f) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  pVar->GetFloat(f);
  return true;
}

bool CVarValueObj::SetFloat(CStrAny const &sVar, float f)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar) {
    pVar = new CVar<float>();
    ReplaceVar(sVar, pVar, true);
  }
  pVar->SetFloat(f);
  return true;
}

CVarObj *CVarValueObj::GetContext(CStrAny const &sVar)
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar || !pVar->ValueHasRTTI())
    return 0;
  return Cast<CVarObj>((CObject *) pVar->GetRef());
}

bool CVarValueObj::GetVar(CStrAny const &sVar, CBaseVar &vDst) const
{
  CBaseVar *pVar = FindVar(sVar);
  if (!pVar)
    return false;
  vDst.SetVar(*pVar);
  return true;
}

bool CVarValueObj::SetVar(CStrAny const &sVar, const CBaseVar &vSrc)
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

bool Set(CStrAny *dst, const int *src)
{
  char chBuf[96];
  _itoa(*src, chBuf, 10);
  *dst = CStrAny(ST_WHOLE, chBuf);
  return true;
}

bool Set(int *dst, const CStrAny *src)
{
  if (src->ZeroTerminated())
    *dst = atoi(src->m_pBuf);
  else {
    CStrAny s = *src;
    *dst = atoi(s.m_pBuf);
  }
  return true;
}

bool Set(CStrAny *dst, const float *src)
{
  char chBuf[96];
  sprintf(chBuf, "%g", *src);
  *dst = CStrAny(ST_WHOLE, chBuf);
  return true;
}

bool Set(float *dst, const CStrAny *src)
{
  int iRes;
  if (src->ZeroTerminated())
    iRes = sscanf(src->m_pBuf, "%f", dst);
  else {
    CStrAny s = *src;
    iRes = sscanf(s.m_pBuf, "%f", dst);
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

CVarObj::CIter *CVarHash::GetIter(CStrAny const &sVar) const
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

CBaseVar *CVarHash::FindVar(CStrAny const &sVar) const
{
  THash::TIter it;
  it = m_Vars.Find(sVar);
  if (!it)
    return 0;
  return it->pVar;
}

bool CVarHash::ReplaceVar(CStrAny const &sVar, CBaseVar *pSrc, bool bAdding)
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
