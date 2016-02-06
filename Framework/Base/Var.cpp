#include "stdafx.h"
#include "Var.h"
#include "VarUtil.h"
#include "Parse.h"

// Var classes RTTI -----------------------------------------------------

CRTTIRegisterer<CBaseVar> g_RegBaseVar;
CRTTIRegisterer<CDummyVar> g_RegDummyVar;

CVarRTTIRegisterer<int> g_RegVarInt;
CVarRTTIRegisterer<float> g_RegVarFloat;
CVarRTTIRegisterer<CStrAny> g_RegVarStrAny;
CVarRTTIRegisterer<uint8_t> g_RegVarBYTE;

CRTTIRegisterer<CVarValueBase<CVarObj> > g_RegVarValueBaseVarObj;
CRTTIRegisterer<CVarRef<CVarObj> > g_RegVarRefVarObj;

// CVarObj --------------------------------------------------------------
CRTTIRegisterer<CVarObj> g_RegVarObj;
CRTTIRegisterer<CVarObj::CIter> g_RegVarObjIter;

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
  DEL(pIt);
  return bRes;
}

bool CVarObj::SetVar(CStrAny const &sVar, const CBaseVar &vSrc)
{
  CIter *pIt = GetIter(sVar);
  if (!pIt)
    return false;
  bool bRes = pIt->SetVar(vSrc);
  DEL(pIt);
  return bRes;
}

// CVarValueObj ---------------------------------------------------------------
CRTTIRegisterer<CVarValueObj> g_RegVarValueObj;

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
    pVar = NEW(CVar<CStrAny>, ());
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
    pVar = NEW(CVar<int>, ());
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
    pVar = NEW(CVar<float>, ());
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
  Parse::Int2Str(*src, chBuf, sizeof(chBuf));
  *dst = CStrAny(ST_WHOLE, chBuf);
  return true;
}

bool Set(int *dst, const CStrAny *src)
{
  bool bRes = Parse::Str2Int(*dst, *src);
  return bRes;
}

bool Set(CStrAny *dst, const float *src)
{
  char chBuf[96];
  Parse::Float2Str(*src, chBuf, sizeof(chBuf));
  *dst = CStrAny(ST_WHOLE, chBuf);
  return true;
}

bool Set(float *dst, const CStrAny *src)
{
  bool bRes = Parse::Str2Float(*dst, *src);
  return bRes;
}

// CVarHash ----------------------------------------------------------------------

CRTTIRegisterer<CVarHash> g_RegVarHash;

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
  return NEW(CIter, (it));
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
      DEL(*it);
      m_Vars.Remove(it);
    }
    return true;
  }

  if (!bAdding) {
    it = m_Vars.Find(sVar);
    bAdding = !it;
  }
  if (bAdding) {
    pVarName = NEW(TVarName, (sVar, pSrc));
    m_Vars.Add(pVarName);
  } else
    it->pVar = pSrc;

  return true;
}
