#include "stdafx.h"
#include "VarUtil.h"
#include "File.h"
#include "Parse.h"

// CVarMerge ------------------------------------------------------------------
CRTTIRegisterer<CVarMerge> g_RegVarMerge;
CRTTIRegisterer<CVarMerge::CIter> g_RegVarMergeIter;

CVarMerge::CIter::CIter(CVarMerge const *pMerge, int iVar, CVarObj::CIter *pIt, int iDirection)
{
  m_pMerge = pMerge;
  m_iVar = iVar;
  if (pIt || m_iVar >= pMerge->m_Vars.m_iCount)
    m_pIt = pIt;
  else {
    m_pIt = pMerge->m_Vars[m_iVar].m_pVar->GetIter(iDirection > 0 ? CStrAny() : GetLastIterConst());
    if (!*m_pIt)
      if (iDirection > 0)
        Next();
      else
        Prev();
  }
}

CVarMerge::CIter &CVarMerge::CIter::Next()
{
  if (!m_pIt)
    return *this;
  do {
    m_pIt->Next();
    if (!*m_pIt)
      if (m_iVar < m_pMerge->m_Vars.m_iCount - 1) {
        m_iVar++;
        DEL(m_pIt);
        m_pIt = m_pMerge->m_Vars[m_iVar].m_pVar->GetIter();
      } else
        break;
  } while (!IsValidPos());
  return *this;
}

CVarMerge::CIter &CVarMerge::CIter::Prev()
{
  if (!m_pIt)
    return *this;
  do {
    m_pIt->Prev();
    if (!*m_pIt)
      if (m_iVar > 0) {
        m_iVar--;
        DEL(m_pIt);
        m_pIt = m_pMerge->m_Vars[m_iVar].m_pVar->GetIter(GetLastIterConst());
      } else
        break;
  } while (!IsValidPos());
  return *this;
}

bool CVarMerge::CIter::IsValidPos()
{
  int i;
  if (!m_pIt || !*m_pIt)
    return false;
  for (i = m_iVar - 1; i >= 0; i--)
    if (m_pMerge->m_Vars[i].m_pVar->FindVar(m_pIt->GetName()))
      return false;
  return true;
}

CVarMerge::CVarMerge(CVarObj *pInherited, CVarObj *pBase, bool bOwnInherited, bool bOwnBase)
{
  ASSERT(pBase && pInherited);
  m_Vars.SetCount(2);
  m_Vars[0].m_pVar = pInherited;
  m_Vars[0].m_bOwned = bOwnInherited;
  m_Vars[1].m_pVar = pBase;
  m_Vars[1].m_bOwned = bOwnBase;
}

CVarMerge::~CVarMerge()
{
  for (int i = 0; i < m_Vars.m_iCount; i++)
    if (m_Vars[i].m_bOwned)
      DEL(m_Vars[i].m_pVar);
}

int CVarMerge::AppendBaseVar(CVarObj *pVar, bool bOwn)
{
  ASSERT(pVar);
  int iVar = m_Vars.m_iCount;
  m_Vars.SetCount(iVar + 1);
  m_Vars[iVar].m_pVar = pVar;
  m_Vars[iVar].m_bOwned = bOwn;
  return iVar;
}

void CVarMerge::RemoveLastBaseVar()
{
  if (m_Vars[m_Vars.m_iCount - 1].m_bOwned)
    DEL(m_Vars[m_Vars.m_iCount - 1].m_pVar);
  m_Vars.SetCount(m_Vars.m_iCount - 1);
}

void CVarMerge::ClearBaseVars()
{
  while (m_Vars.m_iCount)
    RemoveLastBaseVar();
}

CVarObj::CIter *CVarMerge::GetIter(CStrAny const &sVar) const
{
  int i;
  if (!sVar)
    return NEW(CIter, (this, 0, (CIter *) 0));
  if (sVar == GetLastIterConst())
    return NEW(CIter, (this, m_Vars.m_iCount - 1, (CIter *) 0, -1));
  CVarObj::CIter *pIt;
  for (i = 0; i < m_Vars.m_iCount; i++) {
    pIt = m_Vars[i].m_pVar->GetIter(sVar);
    if (pIt)
      return NEW(CIter, (this, i, pIt));
  }
  return 0;
}

CBaseVar *CVarMerge::FindVar(CStrAny const &sVar) const
{
  int i;
  for (i = 0; i < m_Vars.m_iCount; i++) {
    CBaseVar *pVar = m_Vars[i].m_pVar->FindVar(sVar);
    if (pVar)
      return pVar;
  }
  return 0;
}

bool CVarMerge::ReplaceVar(CStrAny const &sVar, CBaseVar *pSrc, bool bAdding)
{
  int i;
  if (!bAdding) {
    for (i = 0; i < m_Vars.m_iCount; i++)
      if (m_Vars[i].m_pVar->FindVar(sVar))
        break;
    bAdding = (i >= m_Vars.m_iCount);
  }
  if (bAdding)
    return m_Vars[0].m_pVar->ReplaceVar(sVar, pSrc, true);
  return m_Vars[i].m_pVar->ReplaceVar(sVar, pSrc, false);
}

// CVarTemplate --------------------------------------------------------------
CRTTIRegisterer<CVarTemplate> g_RegVarTemplate;

void CVarTemplate::Remap(uint8_t *pNewBufBase, uint8_t *pOldBufBase, CVarObj *pDstVars)
{
  CVarObj::CIter *pIt;
  CBaseVar *pDstVal;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    if (pDstVars && pDstVars != m_pVars)
      pDstVal = pDstVars->FindVar(pIt->GetName());
    else
      pDstVal = pIt->GetValue();
    ASSERT(pDstVal);
    pDstVal->SetRef((uint8_t *) pDstVal->GetRef() + (pNewBufBase - pOldBufBase));
  }
  DEL(pIt);
  return;
}

void CVarTemplate::MakeVarCopy(CVarObj *pDstVars, uint8_t *pDstBufBase, uint8_t *pSrcBufBase, bool bAddVars)
{
  CVarObj::CIter *pIt;
  CBaseVar *pOrgVal, *pNewVar;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    pNewVar = pIt->GetValue()->Clone();
    if (pDstBufBase != pSrcBufBase) {
      pNewVar->SetRef((uint8_t *) pNewVar->GetRef() + (pDstBufBase - pSrcBufBase));
      pOrgVal = bAddVars ? 0 : pDstVars->FindVar(pIt->GetName());
      if (pOrgVal)
        pNewVar->SetVar(*pOrgVal);
      else
        if (pSrcBufBase)
          pNewVar->SetVar(*pIt->GetValue());
    }
    pDstVars->ReplaceVar(pIt->GetName(), pNewVar, bAddVars);
  }
  DEL(pIt);
}

void CVarTemplate::ClearVarCopy(CVarObj *pDstVars, uint8_t *pDstBufBase, int iDstBufSize)
{
  CVarObj::CIter *pIt;
  CBaseVar *pDstVal;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    pDstVal = pDstVars->FindVar(pIt->GetName());
    if (!pDstVal)
      continue;
    if (pDstVal->GetRef() < pDstBufBase || pDstVal->GetRef() >= pDstBufBase + iDstBufSize)
      continue;
    pDstVars->ReplaceVar(pIt->GetName(), 0);
  }
  DEL(pIt);
}

void CVarTemplate::CopyValues(CVarObj *pSrcVars)
{
  CVarObj::CIter *pIt;
  CBaseVar *pSrcVal;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    pSrcVal = pSrcVars->FindVar(pIt->GetName());
    if (!pSrcVal)
      continue;
    pIt->SetValue(pSrcVal);
  }
  DEL(pIt);
}

// CVarFile -------------------------------------------------------------------

CVarFile::CVarFile(CFile *pFile)
{
  m_pFile = pFile;
}

CVarFile::~CVarFile()
{
  DEL(m_pFile);
}

bool CVarFile::Read(CVarObj *pVars)
{
  int iSize = (int) m_pFile->GetSize();
  CAutoDeletePtr<char> pBuf(NEWARR(char, iSize + 1));
  CFile::ERRCODE err;
  err = m_pFile->Read(pBuf, iSize);
  if (err)
    return false;
  pBuf[iSize] = 0;
  CStrAny sBuf(ST_PART, pBuf, iSize);
  bool bRes = Set(pVars, &sBuf);
  return bRes;
}

bool CVarFile::Write(CVarObj *pVars, int iIndent)
{
  bool bRes = true;
  CVarObj::CIter *pIter, *pItContext;
  CFile::ERRCODE err;

  for (pIter = pVars->GetIter(); *pIter; pIter->Next()) {
    CStrAny sVal, sRes;
    CVarObj *pContext;
    if (pIter->GetValue()->ValueHasRTTI())
      pContext = Cast<CVarObj>((CObject *) pIter->GetValue()->GetRef());
    else
      pContext = 0;
    if (pContext) {
      sRes = pIter->GetName() + CStrAny(ST_WHOLE, " =  {");
      pItContext = pContext->GetIter();
      if (*pItContext) {
        sRes += CStrAny(ST_WHOLE, "\r\n");
        err = m_pFile->Write((void *) sRes.m_pBuf, sRes.Length());
        bRes = bRes && !err;
        ASSERT(!err);
        bRes &= Write(pContext, iIndent + 2);
        bRes &= WriteIndent(iIndent);
        sRes = "";
      }
      DEL(pItContext);
      sRes += CStrAny(ST_WHOLE, "}\r\n");
      err = m_pFile->Write((void *) sRes.m_pBuf, sRes.Length());
      bRes = bRes && !err;
      ASSERT(!err);
    } else
      if (pIter->GetValue()->GetStr(sVal)) {
        bRes &= WriteIndent(iIndent);
        sRes = pIter->GetName() + CStrAny(ST_WHOLE, " = ") + sVal + CStrAny(ST_WHOLE, "\r\n");
        err = m_pFile->Write((void *) sRes.m_pBuf, sRes.Length());
        bRes = bRes && !err;
        ASSERT(!err);
      }
  }
  DEL(pIter);

  return bRes;
}

bool CVarFile::WriteIndent(int iIndent)
{
  bool bRes = true;
  CFile::ERRCODE err;
  while (iIndent--) {
    err = m_pFile->Write(" ", 1);
    bRes = bRes && !err;
    ASSERT(!err);
  }
  return bRes;
}

// Context var ----------------------------------------------------------------
CVarRTTIRegisterer<CVarHash> g_RegVarVarHash;

CStrAny EncodeValStr(CStrAny const &s)
{
  if (!s)
    return s;

  if (s[0] == '{' && s[s.Length() - 1] == '}')
    return s;

  CStrAny sSpecial(ST_WHOLE, " \n\r\t\\\"");
  CStrAny sTemp;
  bool bEncode;

  bEncode = (s[0] == '"');
  if (!bEncode) {
    sTemp = s;
    sTemp = Parse::ReadUntilChars(sTemp, sSpecial);
    bEncode = sTemp.Length() < s.Length();
  }

  if (!bEncode)
    return s;

  CStrAny sRes(ST_WHOLE, "");
  CStrAny sQuote(ST_WHOLE, "\"");

  sRes = sQuote;

  for (sTemp = s; sTemp.Length(); sTemp >>= 1) {
    char ch = sTemp[0];
    if (ch == '"' || ch == '\\')
      sRes += CStrAny(ST_PART, "\\");
    sRes += CStrAny(ST_PART, &ch, 1);
  }

  sRes += sQuote;

  return sRes;
}

CStrAny DecodeValStr(CStrAny const &s)
{
  if (!s)
    return s;
  if (s[0] != '"')
    return s;
  if (s.Length() < 2 || s[s.Length() - 1] != '"')
    return s;

  CStrAny sRes;
  CStrAny sTemp(s, ST_PART);

  sTemp >>= 1;
  sTemp.m_iLen -= 1;

  while (sTemp.Length()) {
    if (sTemp[0] == '\\') {
      sTemp >>= 1;
      if (!sTemp)
        break;
    }
    sRes += CStrAny(ST_PART, sTemp.m_pBuf, 1);

    sTemp >>= 1;
  }

  return sRes;
}

bool Set(CStrAny *dst, CVarObj const *src)
{
  bool bRes = true;
  CStrAny sRes, sVal;
  sRes = "{ ";

  CVarObj::CIter *pIt;
  for (pIt = src->GetIter(); *pIt; pIt->Next()) {
    bool bConverted = pIt->GetValue()->GetStr(sVal);
    bRes &= bConverted;
    if (!bConverted)
      continue;
    sRes = sRes + CStrAny(ST_WHOLE, "\r\n") + EncodeValStr(pIt->GetName()) + CStrAny(ST_WHOLE, " = ") + EncodeValStr(sVal);
  }

  if (sRes.Length() > 3)
    sRes += CStrAny(ST_WHOLE, "\r\n");

  sRes += CStrAny(ST_WHOLE, " }");
  *dst = sRes;
  return bRes;
}

bool Set(CVarObj *dst, CStrAny const *src)
{
  CStrAny sSrc(*src, ST_PART), sBlock;
  int iInd;
  bool bRes = true;

  static Parse::TDelimiterBlock arrDelim[] = {
    { CStrAny(ST_WHOLE, "{"), CStrAny(ST_WHOLE, "}"), true },
    { CStrAny(ST_WHOLE, "\""), CStrAny(ST_WHOLE, "\""), false },
  };

  Parse::ReadWhitespace(sSrc);

  sBlock = Parse::MatchDelimiters(sSrc, arrDelim, ARRSIZE(arrDelim), &iInd);
  if (!!sBlock) {
    if (iInd)
      return false;
    Parse::ReadWhitespace(sSrc);
    bRes &= !sSrc;

    sSrc.m_pBuf = sBlock.m_pBuf + 1;
    sSrc.m_iLen = sBlock.m_iLen - 2;
  }

  while (!!sSrc) {
    CStrAny sStart;
    sStart = Parse::ReadUntilChar(sSrc, '=');
    sStart = Parse::TrimWhitespace(sStart);
    if (!sStart)
      continue;
    if (!sSrc)
      bRes = false;
    CStrAny sVarName(DecodeValStr(sStart), ST_CONST);
    Parse::ReadChar(sSrc, '=');
    Parse::ReadWhitespace(sSrc);
    sBlock = Parse::MatchDelimiters(sSrc, arrDelim, ARRSIZE(arrDelim), &iInd);
    if (!sBlock)
      sBlock = Parse::ReadUntilWhitespace(sSrc);
    CStrAny sVal = DecodeValStr(sBlock);
    CBaseVar *pVar = 0;
    if (!!sBlock && sBlock[0] != '"') {
      CStrAny sRest(sVal, ST_PART);
      CStrAny sNum = Parse::ReadFloat(sRest);
      Parse::ReadWhitespace(sRest);
      if (!!sNum && !sRest) {
        sRest = sNum;
        Parse::ReadUntilChar(sRest, '.');
        if (!!sRest)
          pVar = NEW(CVar<float>, ());
        else
          pVar = NEW(CVar<int>, ());
      } else
        if (sBlock[0] == '{')
          pVar = NEW(CVar<CVarHash>, ());
    }
    if (!pVar)
      pVar = NEW(CVar<CStrAny>, ());
    bRes &= pVar->SetStr(sVal);
    dst->ReplaceVar(sVarName, pVar);
  }

  return bRes;
}

bool Set(CVarObj *dst, CVarObj const *src)
{
  bool bRes = true;
  CVarObj::CIter *pIt;
  for (pIt = src->GetIter(); *pIt; pIt->Next())
    bRes &= dst->SetVar(pIt->GetName(), *pIt->GetValue());
  DEL(pIt);
  return bRes;
}

bool SetValue(CVarObj *val, CBaseVar const *vSrc)
{
  if (!vSrc->ValueHasRTTI())
    return false;

  bool bRes;
  const CVarObj *pVO = Cast<CVarObj>((CObject *) vSrc->GetRef());
  if (pVO) {
    bRes = true;
    CVarObj::CIter *pIt;
    for (pIt = pVO->GetIter(); *pIt; pIt->Next())
      bRes &= val->SetVar(pIt->GetName(), *pIt->GetValue());
    DEL(pIt);
  } else
    bRes = false;
  return bRes;
}
