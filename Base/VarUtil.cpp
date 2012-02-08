#include "stdafx.h"
#include "VarUtil.h"
#include "File.h"
#include "Parse.h"

// CVarMerge ------------------------------------------------------------------
IMPRTTI_NOCREATE(CVarMerge, CVarObj)
IMPRTTI_NOCREATE(CVarMerge::CIter, CVarObj::CIter)

CVarMerge::CIter::CIter(CVarMerge const *pMerge, int iVar, CVarObj::CIter *pIt, int iDirection)
{
  m_pMerge = pMerge;
  m_iVar = iVar;
  if (pIt || m_iVar >= pMerge->m_Vars.m_iCount)
    m_pIt = pIt;
  else {
    m_pIt = pMerge->m_Vars[m_iVar].m_pVar->GetIter(iDirection > 0 ? CStrPart() : GetLastIterConst());
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
        delete m_pIt;
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
        delete m_pIt;
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
      delete m_Vars[i].m_pVar;
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
    delete m_Vars[m_Vars.m_iCount - 1].m_pVar;
  m_Vars.SetCount(m_Vars.m_iCount - 1);
}

void CVarMerge::ClearBaseVars()
{
  while (m_Vars.m_iCount)
    RemoveLastBaseVar();
}

CVarObj::CIter *CVarMerge::GetIter(const CStrBase &sVar) const
{
  int i;
  if (!sVar) 
    return new CIter(this, 0, 0);
  if (sVar == GetLastIterConst()) 
    return new CIter(this, m_Vars.m_iCount - 1, 0, -1);
  CVarObj::CIter *pIt;
  for (i = 0; i < m_Vars.m_iCount; i++) {
    pIt = m_Vars[i].m_pVar->GetIter(sVar);
    if (pIt)
      return new CIter(this, i, pIt);
  }
  return 0;
}

CBaseVar *CVarMerge::FindVar(const CStrBase &sVar) const
{
  int i;
  for (i = 0; i < m_Vars.m_iCount; i++) {
    CBaseVar *pVar = m_Vars[i].m_pVar->FindVar(sVar);
    if (pVar)
      return pVar;
  }
  return 0;
}

bool CVarMerge::ReplaceVar(const CStrBase &sVar, CBaseVar *pSrc, bool bAdding)
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
IMPRTTI(CVarTemplate, CObject)

void CVarTemplate::Remap(BYTE *pNewBufBase, BYTE *pOldBufBase, CVarObj *pDstVars)
{
  CVarObj::CIter *pIt;
  CBaseVar *pDstVal;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    if (pDstVars && pDstVars != m_pVars)
      pDstVal = pDstVars->FindVar(pIt->GetName());
    else
      pDstVal = pIt->GetValue();
    ASSERT(pDstVal);
    pDstVal->SetRef((BYTE *) pDstVal->GetRef() + (pNewBufBase - pOldBufBase));
  }
  delete pIt;
  return;
}

void CVarTemplate::MakeVarCopy(CVarObj *pDstVars, BYTE *pDstBufBase, BYTE *pSrcBufBase, bool bAddVars)
{
  CVarObj::CIter *pIt;
  CBaseVar *pOrgVal, *pNewVar;
  for (pIt = m_pVars->GetIter(); *pIt; pIt->Next()) {
    pNewVar = pIt->GetValue()->Clone();
    if (pDstBufBase != pSrcBufBase) {
      pNewVar->SetRef((BYTE *) pNewVar->GetRef() + (pDstBufBase - pSrcBufBase));
      pOrgVal = bAddVars ? 0 : pDstVars->FindVar(pIt->GetName());
      if (pOrgVal)
        pNewVar->SetVar(*pOrgVal);
      else
        if (pSrcBufBase)
          pNewVar->SetVar(*pIt->GetValue());
    }
    pDstVars->ReplaceVar(pIt->GetName(), pNewVar, bAddVars);
  }
  delete pIt;
}

void CVarTemplate::ClearVarCopy(CVarObj *pDstVars, BYTE *pDstBufBase, int iDstBufSize)
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
  delete pIt;
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
  delete pIt;
}

// CVarFile -------------------------------------------------------------------

CVarFile::CVarFile(CFileBase *pFile)
{
  m_pFile = pFile;
}

CVarFile::~CVarFile()
{
  delete m_pFile;
}

bool CVarFile::Read(CVarObj *pVars)
{
  int iSize = (int) m_pFile->GetSize();
  CAutoDeletePtr<char> pBuf(new char[iSize + 1]);
  CFile::ERRCODE err;
  err = m_pFile->Read(pBuf, iSize);
  if (err)
    return false;
  pBuf[iSize] = 0;
  CStrPart sBuf(pBuf, iSize);
  bool bRes = Set(pVars, &sBuf);
  return bRes;
}

bool CVarFile::Write(CVarObj *pVars, int iIndent)
{
  bool bRes = true;
  CVarObj::CIter *pIter, *pItContext;
  CFile::ERRCODE err;

  for (pIter = pVars->GetIter(); *pIter; pIter->Next()) {
    CStr sVal, sRes;
    CVarObj *pContext;
    if (pIter->GetValue()->ValueHasRTTI())
      pContext = Cast<CVarObj>((CObject *) pIter->GetValue()->GetRef());
    else
      pContext = 0;
    if (pContext) {
      sRes = pIter->GetName() + CStrPart(" =  {");
      pItContext = pContext->GetIter();
      if (*pItContext) {
        sRes += CStrPart("\r\n");
        err = m_pFile->Write((void *) (const char *) sRes, sRes.Length());
        bRes = bRes && !err;
        ASSERT(!err);
        bRes &= Write(pContext, iIndent + 2);
        bRes &= WriteIndent(iIndent);
        sRes = "";
      } 
      delete pItContext;
      sRes += CStrPart("}\r\n");
      err = m_pFile->Write((void *) (const char *) sRes, sRes.Length());
      bRes = bRes && !err;
      ASSERT(!err);
    } else
      if (pIter->GetValue()->GetStr(sVal)) {
        bRes &= WriteIndent(iIndent);
        sRes = pIter->GetName() + CStrPart(" = ") + sVal + CStrPart("\r\n");
        err = m_pFile->Write((void *) (const char *) sRes, sRes.Length());
        bRes = bRes && !err;
        ASSERT(!err);
      }
  }
  delete pIter;

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
IMP_VAR_RTTI(CVarHash)

CStr EncodeValStr(const CStrBase &s)
{
  if (!s)
    return s;

  if (s[0] == '{' && s[s.Length() - 1] == '}')
    return s;

  CStrPart sSpecial(" \n\r\t\\\"");
  CStrPart sTemp;
  bool bEncode;

  bEncode = (s[0] == '"');
  if (!bEncode) {
    sTemp = s;
    sTemp = Parse::ReadUntilChars(sTemp, sSpecial);
    bEncode = sTemp.Length() < s.Length();
  }
  
  if (!bEncode)
    return s;

  CStr sRes;
  CStrPart sQuote = CStrPart("\"");

  sRes = sQuote;
  
  for (sTemp = s; sTemp.Length(); sTemp += 1) {
    char ch = sTemp[0];
    if (ch == '"' || ch == '\\')
      sRes += CStrPart("\\");
    sRes += CStrPart(&ch, 1);
  }

  sRes += sQuote;

  return sRes;
}

CStr DecodeValStr(const CStrBase &s)
{
  if (!s)
    return s;
  if (s[0] != '"')
    return s;
  if (s.Length() < 2 || s[s.Length() - 1] != '"')
    return s;

  CStr sRes;
  CStrPart sTemp(s);

  sTemp += 1;
  sTemp.m_iLen -= 1;

  while (sTemp.Length()) {
    if (sTemp[0] == '\\') {
      sTemp += 1;
      if (!sTemp)
        break;
    }
    sRes += CStrPart(sTemp, 1);

    sTemp += 1;
  }

  return sRes;
}

bool Set(CStrBase *dst, CVarObj const *src)
{
  bool bRes = true;
  CStr sRes, sVal;
  sRes = "{ ";

  CVarObj::CIter *pIt;
  for (pIt = src->GetIter(); *pIt; pIt->Next()) {
    bool bConverted = pIt->GetValue()->GetStr(sVal);
    bRes &= bConverted;
    if (!bConverted)
      continue;
    sRes = sRes + CStrPart("\r\n") + EncodeValStr(pIt->GetName()) + CStrPart(" = ") + EncodeValStr(sVal);
  }

  if (sRes.Length() > 3)
    sRes += CStrPart("\r\n");

  sRes += CStrPart(" }");
  dst->Assign(sRes);
  return bRes;
}

bool Set(CVarObj *dst, CStrBase const *src)
{
  CStrPart sSrc(*src), sBlock;
  int iInd;
  bool bRes = true;

  static Parse::TDelimiterBlock arrDelim[] = {
    { "{", "}", true },
    { "\"", "\"", false },
  };

  Parse::ReadWhitespace(sSrc);

  sBlock = Parse::MatchDelimiters(sSrc, arrDelim, ARRSIZE(arrDelim), &iInd);
  if (sBlock) {
    if (iInd)
      return false;
    Parse::ReadWhitespace(sSrc);
    bRes &= !sSrc;

    sSrc.m_pBuf = sBlock.m_pBuf + 1;
    sSrc.m_iLen = sBlock.m_iLen - 2;
  }

  while (!!sSrc) {
    CStrPart sStart;
    sStart = Parse::ReadUntilChar(sSrc, '=');
    sStart = Parse::TrimWhitespace(sStart);
    if (!sStart)
      continue;
    if (!sSrc)
      bRes = false;
    CStrConst sVarName = DecodeValStr(sStart); 
    Parse::ReadChar(sSrc, '=');
    Parse::ReadWhitespace(sSrc);
    sBlock = Parse::MatchDelimiters(sSrc, arrDelim, ARRSIZE(arrDelim), &iInd);
    if (!sBlock)
      sBlock = Parse::ReadUntilWhitespace(sSrc);
    CStr sVal = DecodeValStr(sBlock);
    CBaseVar *pVar = 0;
    if (!!sBlock && sBlock[0] != '"') {
      CStrPart sRest(sVal);
      CStrPart sNum = Parse::ReadFloat(sRest);
      Parse::ReadWhitespace(sRest);
      if (!!sNum && !sRest) {
        sRest = sNum;
        Parse::ReadUntilChar(sRest, '.');
        if (!!sRest) 
          pVar = new CVar<float>();
        else
          pVar = new CVar<int>();
      } else
        if (sBlock[0] == '{')
          pVar = new CVar<CVarHash>();
    }
    if (!pVar)
      pVar = new CVar<CStr>();
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
  delete pIt;
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
    delete pIt;
  } else
    bRes = false;
  return bRes;
}
