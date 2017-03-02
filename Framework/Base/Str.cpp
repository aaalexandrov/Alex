#include "stdafx.h"
#include "Str.h"
#include "Parse.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// CStrHeader ---------------------------------------------------------

CStrHeader *CStrHeader::Get(const char *pSrc, int iLen, bool bInRepository)
{
  if (!pSrc) {
    if (iLen < 0)
      return 0;
    bInRepository = bInRepository && iLen == 0;
  }
  if (iLen < 0)
    iLen = (int) strlen(pSrc);
  if (bInRepository) {
    THash::TIter it = GetRepository().Find(CStrAny(ST_PART, pSrc, iLen));
    if (it)
      return *it;
  }
  CStrHeader *pHeader = CStrHeader::Alloc(iLen);
  pHeader->Init(iLen, pSrc);
  if (bInRepository) {
    pHeader->m_bInRepository = true;
    GetRepository().Add(pHeader);
  }
  return pHeader;
}

CStrHeader const *CStrHeader::GetUnique() const
{
  if (GetRef() <= 1)
    return this;
  CStrHeader *pNewHeader = CStrHeader::Alloc(m_iLen);
  pNewHeader->Init(m_iLen, (const char *) (this + 1));
  if (m_bHashInitialized) {
    pNewHeader->m_bHashInitialized = true;
    pNewHeader->m_uiHash = m_uiHash;
  }
  return pNewHeader;
}

bool CStrHeader::CheckRepository()
{
  bool bRes;
  THash::TIter it;

  bRes = true;
  for (it = GetRepository(); it; ++it) {
    ASSERT(it->m_bInRepository && it->m_bHashInitialized);
    bRes &= it->m_bInRepository && it->m_bHashInitialized;
  }
  return bRes;
}

// CStrAny --------------------------------------------------------------------

CStrAny::CStrAny(EStrType eType, char const *pStr, int iLen)
{
  Init(eType, pStr, iLen);
}

CStrAny::CStrAny(EStrType eType, int i)
{
  char chBuf[32];
  Parse::Int2Str(i, chBuf, ARRSIZE(chBuf), 10);
  eType = (EStrType) (eType | ST_HASHEADER | ST_ZEROTERMINATED);
  Init(eType, chBuf, -1);
}

CStrAny::CStrAny(EStrType eType, float f)
{
  char chBuf[32];
  Parse::Float2Str(f, chBuf, ARRSIZE(chBuf), 10);
  eType = (EStrType) (eType | ST_HASHEADER | ST_ZEROTERMINATED);
  Init(eType, chBuf, -1);
}

CStrAny::CStrAny(EStrType eType, char c, int iRepeatCount)
{
  Init(eType, "", iRepeatCount);
  memset((char *) m_pBuf, c, iRepeatCount * sizeof(char));
  if (m_bZeroTerminated)
    ((char *) m_pBuf)[iRepeatCount] = 0;
}

CStrAny::CStrAny(CStrAny const &s, EStrType eType)
{
  Init(eType, s);
}

CStrAny::CStrAny(CStrHeader const *pHeader)
{
  Init(pHeader);
}

CStrAny::~CStrAny()
{
  Done();
}

void CStrAny::Init(EStrType eType, char const *pStr, int iLen)
{
  if (!pStr) {
    if (iLen <= 0) {
      m_pBuf = 0;
      m_iLen = 0;
      m_bHasHeader = false;
      m_bZeroTerminated = false;
      return;
    }
    eType = (EStrType) (eType & ~ST_INREPOSITORY);
  }
  if (iLen < 0) {
    eType = (EStrType) (eType | ST_ZEROTERMINATED);
    iLen = strlen(pStr);
  }
  m_bZeroTerminated = !!(eType & (ST_ZEROTERMINATED | ST_HASHEADER));
  m_bHasHeader = !!(eType & ST_HASHEADER);
  m_iLen = iLen;
  if (m_bHasHeader) {
    CStrHeader *pHeader = CStrHeader::Get(pStr, iLen, !!(eType & ST_INREPOSITORY));
    pHeader->Acquire();
    m_pBuf = (char const *) (pHeader + 1);
  } else
    m_pBuf = pStr;
}

void CStrAny::Init(EStrType eType, CStrAny const &s)
{
  if (eType & ST_PRESERVETYPE) {
    m_pBuf = s.m_pBuf;
    m_iLen = s.m_iLen;
    m_bHasHeader = s.m_bHasHeader;
    m_bZeroTerminated = s.m_bZeroTerminated;
    if (m_bHasHeader)
      GetHeader()->Acquire();
  } else
    Init(eType, s.m_pBuf, s.m_iLen);
}

void CStrAny::Init(CStrHeader const *pHeader)
{
  ASSERT(pHeader);
  pHeader->Acquire();
  m_pBuf = (char const *) (pHeader + 1);
  m_iLen = pHeader->m_iLen;
  m_bHasHeader = true;
  m_bZeroTerminated = true;
}

void CStrAny::Done()
{
  if (m_bHasHeader)
    GetHeader()->Release();
}

CStrAny &CStrAny::MakeUnique()
{
  if (m_bHasHeader) {
    CStrHeader const *pHeader, *pNewHeader;
    pHeader = GetHeader();
    pNewHeader = pHeader->GetUnique();
    pNewHeader->Acquire();
    pHeader->Release();
    m_pBuf = (char const *) (pNewHeader + 1);
  } else
    Init(ST_STR, m_pBuf, m_iLen);
  return *this;
}

CStrAny &CStrAny::AssureHasHeader()
{
  if (m_bHasHeader)
    return *this;
  ASSERT(m_pBuf || !m_iLen);
  if (!m_pBuf)
    m_pBuf = "";
  Init(ST_STR, m_pBuf, m_iLen);
  return *this;
}

CStrAny &CStrAny::AssureInRepository()
{
  if (!m_pBuf)
    return *this;
  if (m_bHasHeader) {
    CStrHeader *pHeader = (CStrHeader*) GetHeader();
    CStrHeader *pRepoHeader = pHeader->AssureInRepository();
    pRepoHeader->Acquire();
    pHeader->Release();
    m_pBuf = (char const *) (pRepoHeader + 1);
  } else
    Init(ST_CONST, m_pBuf, m_iLen);
  return *this;
}

CStrAny &CStrAny::operator =(CStrAny const &s)
{
  CStrHeader const *pHeader = GetHeader();
  Init(ST_SAME, s);
  if (pHeader)
    pHeader->Release();
  return *this;
}

CStrAny &CStrAny::operator =(const char *pStr)
{
  Done();
  Init(ST_STR, pStr, -1);
  return *this;
}

CStrHeader *CStrAny::GetConcatenationHeader(CStrAny const &s, bool bForceNew) const
{
  ASSERT((m_pBuf || m_iLen) && s.m_iLen);
  CStrHeader *pHeader = (CStrHeader *) GetHeader();
  ASSERT(!pHeader || pHeader->m_iLen == m_iLen);
  if (bForceNew || !pHeader || pHeader->GetRef() > 1 || pHeader->m_bInRepository || CStrHeader::GetMaxLen(m_iLen) < m_iLen + s.m_iLen) {
    pHeader = CStrHeader::Alloc(m_iLen + s.m_iLen);
    pHeader->Init(m_iLen + s.m_iLen, 0);
    memcpy(pHeader + 1, m_pBuf, m_iLen * sizeof(char));
  } else {
    pHeader->m_iLen += s.m_iLen;
    pHeader->m_bHashInitialized = false;
  }
  memcpy((char *) (pHeader + 1) + m_iLen * sizeof(char), s.m_pBuf, s.m_iLen * sizeof(char));
  ((char *) (pHeader + 1)) [pHeader->m_iLen] = 0;
  return pHeader;
}

CStrAny CStrAny::operator +(CStrAny const &s) const
{
  if (m_iLen == 0)
    return s;
  if (s.m_iLen == 0)
    return *this;
  CStrHeader *pHeader = GetConcatenationHeader(s, true);
  return CStrAny(pHeader);
}

CStrAny &CStrAny::operator +=(CStrAny const &s)
{
  if (!s.m_iLen)
    return *this;
  if (!m_pBuf) {
    *this = s;
    return *this;
  }
  CStrHeader *pOrgHeader = (CStrHeader *) GetHeader();
  CStrHeader *pHeader = GetConcatenationHeader(s, false);
  if (pHeader != pOrgHeader) {
    Init(pHeader);
    if (pOrgHeader)
      pOrgHeader->Release();
  } else {
    m_iLen = pHeader->m_iLen;
  }
  return *this;
}

void CStrAny::GetUnionBeginEnd(CStrAny const &s, char const *&pStart, char const *&pEnd, bool &bZeroTerminated) const
{
  if (!m_pBuf) {
    pStart = s.m_pBuf;
    pEnd = s.m_pBuf + s.m_iLen;
    bZeroTerminated = !!s.m_bZeroTerminated;
  } else
    if (!s.m_pBuf) {
      pStart = m_pBuf;
      pEnd = m_pBuf + m_iLen;
      bZeroTerminated = !!m_bZeroTerminated;
    } else {
      pStart = Util::Min(m_pBuf, s.m_pBuf);
      if (m_pBuf + m_iLen >= s.m_pBuf + s.m_iLen) {
        pEnd = m_pBuf + m_iLen;
        bZeroTerminated = m_bZeroTerminated || (m_pBuf + m_iLen == s.m_pBuf + s.m_iLen && s.m_bZeroTerminated);
      } else {
        pEnd = s.m_pBuf + s.m_iLen;
        bZeroTerminated = !!s.m_bZeroTerminated;
      }
    }
}

CStrAny CStrAny::operator >>(CStrAny const &s) const
{
  char const *pStart, *pEnd;
  bool bZeroTerminated;
  GetUnionBeginEnd(s, pStart, pEnd, bZeroTerminated);
  return CStrAny(bZeroTerminated ? ST_WHOLE : ST_PART, pStart, pEnd - pStart);
}

CStrAny &CStrAny::operator >>=(CStrAny const &s)
{
  ASSERT(!m_bHasHeader);
  if (m_bHasHeader)
    return *this;
  char const *pStart, *pEnd;
  bool bZeroTerminated;
  GetUnionBeginEnd(s, pStart, pEnd, bZeroTerminated);
  m_pBuf = pStart;
  m_iLen = pEnd - pStart;
  m_bZeroTerminated = bZeroTerminated;
  return *this;
}

CStrAny CStrAny::operator >>(int i) const
{
  if (i > m_iLen)
    i = m_iLen;
  return SubStr(i, m_iLen);
}

CStrAny &CStrAny::operator >>=(int i)
{
  ASSERT(!m_bHasHeader);
  if (m_bHasHeader)
    return *this;
  if (i > m_iLen)
    i = m_iLen;
  m_pBuf += i;
  m_iLen -= i;
  return *this;
}

CStrAny CStrAny::SubStr(int iStart, int iEnd, EStrType eType) const
{
  iStart = Util::Bound(iStart, 0, m_iLen);
  if (iEnd < 0)
    iEnd = m_iLen;
  else
    iEnd = Util::Bound(iEnd, iStart, m_iLen);
  if (iStart == 0 && iEnd == m_iLen && eType & ST_SAME)
    return *this;
  if (iEnd == m_iLen && m_bZeroTerminated)
    eType = (EStrType) (eType | ST_ZEROTERMINATED);
  return CStrAny(eType, m_pBuf + iStart, iEnd - iStart);
}

int CStrAny::Find(CStrAny const &s, int iStartPos) const
{
  int iLen, iLen1, iMatch, iStart;
  iLen = Length();
  iLen1 = s.Length();
  iMatch = 0;
  for (iStart = iStartPos; iStart < iLen - iLen1; iStart++) {
    for (iMatch = 0; iMatch < iLen1 && m_pBuf[iStart + iMatch] == s.m_pBuf[iMatch]; iMatch++);
    if (iMatch >= iLen1)
      return iStart;
  }
  return -1;
}

int CStrAny::IFind(CStrAny const &s, int iStartPos) const
{
  int iLen, iLen1, iMatch, iStart;
  iLen = Length();
  iLen1 = s.Length();
  iMatch = 0;
  for (iStart = iStartPos; iStart < iLen - iLen1; iStart++) {
    for (iMatch = 0; iMatch < iLen1 && tolower(m_pBuf[iStart + iMatch]) == tolower(s.m_pBuf[iMatch]); iMatch++);
    if (iMatch >= iLen1)
      return iStart;
  }
  return -1;
}

int CStrAny::Find(char ch, int iStartPos) const
{
  while (!EndsAt(iStartPos) && m_pBuf[iStartPos] != ch)
    iStartPos++;
  if (EndsAt(iStartPos))
    return -1;
  return iStartPos;
}

int CStrAny::IFind(char ch, int iStartPos) const
{
  ch = tolower(ch);
  while (!EndsAt(iStartPos) && tolower(m_pBuf[iStartPos]) != ch)
    iStartPos++;
  if (EndsAt(iStartPos))
    return -1;
  return iStartPos;
}

int CStrAny::Cmp(CStrAny const &s) const
{
  if (m_pBuf == s.m_pBuf || !m_iLen || !s.m_iLen)
    return m_iLen - s.m_iLen;

  int i = 0;
  while (!EndsAt(i) && !s.EndsAt(i) && m_pBuf[i] == s.m_pBuf[i])
    i++;
  if (EndsAt(i)) {
    if (s.EndsAt(i))
      return 0;
    else
      return -1;
  }
  if (s.EndsAt(i))
    return 1;
  return (int) m_pBuf[i] - (int) s.m_pBuf[i];
}

int CStrAny::ICmp(CStrAny const &s) const
{
  if (m_pBuf == s.m_pBuf || !m_iLen || !s.m_iLen)
    return m_iLen - s.m_iLen;

  int i = 0;
  char ch1, ch2;
  while (!EndsAt(i) && !s.EndsAt(i) && (ch1 = tolower(m_pBuf[i])) == (ch2 = tolower(s.m_pBuf[i])))
    i++;
  if (EndsAt(i)) {
    if (s.EndsAt(i))
      return 0;
    else
      return -1;
  }
  if (s.EndsAt(i))
    return 1;
  return (int) ch1 - (int) ch2;
}

