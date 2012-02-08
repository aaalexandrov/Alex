#include "stdafx.h"
#include "Str.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// CStrBase -----------------------------------------------------------

CStrPart CStrBase::SubStr(int iStart, int iEnd) const
{
  int iLen = Length();
  if (iStart < 0)
    iStart = 0;
  if (iStart > iLen)
    iStart = iLen;
  if (iEnd > iLen || iEnd < 0)
    iEnd = iLen;
  if (iStart > iEnd)
    iEnd = iStart;
  iLen = iEnd - iStart;
  return CStrPart(m_pBuf + iStart, iLen);
}

int CStrBase::Find(const CStrBase &s, int iStartPos) const
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

int CStrBase::IFind(const CStrBase &s, int iStartPos) const
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


int CStrBase::Find(char ch, int iStartPos) const
{
  while (!EndsAt(iStartPos) && m_pBuf[iStartPos] != ch)
    iStartPos++;
  if (EndsAt(iStartPos))
    return -1;
  return iStartPos;
}

int CStrBase::IFind(char ch, int iStartPos) const
{
  ch = tolower(ch);
  while (!EndsAt(iStartPos) && tolower(m_pBuf[iStartPos]) != ch)
    iStartPos++;
  if (EndsAt(iStartPos))
    return -1;
  return iStartPos;
}

int CStrBase::Cmp(const CStrBase &s) const
{
  if (EndsAt(0)) 
    if (s.EndsAt(0))
      return 0;
    else
      return -1;
  if (s.EndsAt(0))
    return 1;

  const THeader *pHeader, *pHeaderS;
  pHeader = GetHeader();
  pHeaderS = s.GetHeader();
  if (pHeader && pHeaderS) 
    if (pHeader && pHeaderS && pHeader->m_bInRepository && pHeaderS->m_bInRepository && pHeader == pHeaderS)
      return 0;

  int i = 0;
  while (!EndsAt(i) && !s.EndsAt(i) && m_pBuf[i] == s.m_pBuf[i])
    i++;
  if (EndsAt(i))
    if (s.EndsAt(i))
      return 0;
    else
      return -1;
  if (s.EndsAt(i))
    return 1;
  return (int) m_pBuf[i] - (int) s.m_pBuf[i];
}

int CStrBase::ICmp(const CStrBase &s) const
{
  if (EndsAt(0)) 
    if (s.EndsAt(0))
      return 0;
    else
      return -1;
  if (s.EndsAt(0))
    return 1;

  const THeader *pHeader, *pHeaderS;
  pHeader = GetHeader();
  pHeaderS = s.GetHeader();
  if (pHeader && pHeaderS) 
    if (pHeader && pHeaderS && pHeader->m_bInRepository && pHeaderS->m_bInRepository && pHeader == pHeaderS)
      return 0;

  int i = 0;
  char ch1, ch2;
  while (!EndsAt(i) && !s.EndsAt(i) && (ch1 = tolower(m_pBuf[i])) == (ch2 = tolower(s.m_pBuf[i])))
    i++;
  if (EndsAt(i))
    if (s.EndsAt(i))
      return 0;
    else
      return -1;
  if (s.EndsAt(i))
    return 1;
  return (int) ch1 - (int) ch2;
}

// CStr ---------------------------------------------------------------

CStr::CStr(const char *pStr, int iLen)
{
  Init(pStr, iLen);
}

CStr::CStr(int i)
{
  static char buf[96];
  _itoa(i, buf, 10);
  int iLen = (int) strlen(buf);
  THeader *pHeader = THeader::Alloc(iLen);
  pHeader->Init(iLen, buf);
  m_pBuf = (const char *) (pHeader + 1);
}

CStr::~CStr()
{
  const THeader *pHeader = GetHeader();
  if (pHeader)
    pHeader->Release();
}

void CStr::Construct(const CStrBase &s)
{
  const THeader *pHeader = s.GetHeader();
  if (pHeader) {
    pHeader->Acquire();
    m_pBuf = (const char *) (pHeader + 1);
  } else 
    Init(s.m_pBuf, s.Length());
}


void CStr::Init(const char *pStr, int iLen)
{
  ASSERT(pStr || iLen <= 0);
  if (iLen < 0)
    iLen = pStr ? (int) strlen(pStr) : 0;
  if (pStr) {
    THeader *pHeader = THeader::Alloc(iLen);
    pHeader->Init(iLen, pStr);
    m_pBuf = (const char *) (pHeader + 1);
  } else
    m_pBuf = 0;
}

CStr &CStr::operator =(const CStrBase &s)
{
  const THeader *pHeader = GetHeader();
  const THeader *pHeaderS = s.GetHeader();
  if (pHeaderS) {
    pHeaderS->Acquire();
    m_pBuf = s.m_pBuf;
  } else
    Init(s.m_pBuf, s.Length());
  if (pHeader)
    pHeader->Release();
  return *this;
}

CStr &CStr::operator =(const char *pStr)
{
  const THeader *pHeader = GetHeader();
  if (pHeader)
    pHeader->Release();
  Init(pStr);
  return *this;
}

CStr CStr::operator +(const CStrBase &s) const
{
  int iLen, iLen1;
  THeader *pHeader;
  iLen = Length();
  iLen1 = s.Length();
  pHeader = THeader::Alloc(iLen + iLen1);
  pHeader->Init(iLen + iLen1, 0);
  memcpy(pHeader + 1, m_pBuf, iLen * sizeof(char));
  memcpy((char *) (pHeader + 1) + iLen, s.m_pBuf, iLen1 * sizeof(char));
  ((char *) (pHeader + 1))[iLen + iLen1] = 0;
  CStr sRes;
  sRes.m_pBuf = (const char *) (pHeader + 1);
  return sRes;
}

CStr &CStr::operator +=(const CStrBase &s)    
{ 
  int iLen, iLen1;
  THeader *pHeader;
  iLen1 = s.Length();
  if (!iLen1)
    return *this;
  pHeader = (THeader *) GetHeader();
  iLen = Length();
  if (!pHeader || THeader::GetMaxLen(iLen) < iLen + iLen1) {
    THeader *pNewHeader = THeader::Alloc(iLen + iLen1);
    pNewHeader->Init(iLen + iLen1, 0);
    memcpy(pNewHeader + 1, pHeader + 1, iLen * sizeof(char));
    if (pHeader)
      pHeader->Release();
    pHeader = pNewHeader;
    m_pBuf = (const char *) (pHeader + 1);
  } else
    pHeader->m_iLen += iLen1;
  memcpy((char *) (pHeader + 1) + iLen, s.m_pBuf, iLen1 * sizeof(char));
  ((char *) m_pBuf)[iLen + iLen1] = 0;
  return *this; 
}

void CStr::MakeUnique()
{
  const THeader *pHeader = GetHeader();
  if (!pHeader || pHeader->GetRef() <= 1)
    return;
  THeader *pNewHeader = THeader::Alloc(pHeader->m_iLen);
  pNewHeader->Init(pHeader->m_iLen, (const char *) (pHeader + 1));
  if (pHeader->m_bHashInitialized) {
    pNewHeader->m_bHashInitialized = true;
    pNewHeader->m_uiHash = pHeader->m_uiHash;
  }
  pHeader->Release();
  m_pBuf = (const char *) (pNewHeader + 1);
}

CStr &CStr::ToUpper()
{
  MakeUnique();
  int i;
  for (i = 0; !EndsAt(i); i++)
    ((char *) m_pBuf)[i] = toupper(m_pBuf[i]);
  return *this;
}

CStr &CStr::ToLower()
{
  MakeUnique();
  int i;
  for (i = 0; !EndsAt(i); i++)
    ((char *) m_pBuf)[i] = tolower(m_pBuf[i]);
  return *this;
}

// CStrPart -----------------------------------------------------------

CStrPart::CStrPart(const char *pStr, int iLen)
{
  Set(pStr, iLen);
}

CStrPart &CStrPart::Set(const char *pStr, int iLen)
{
  ASSERT(pStr || iLen <= 0);
  if (iLen < 0)
    iLen = pStr ? (int) strlen(pStr) : 0;
  m_pBuf = (char *) pStr;
  m_iLen = iLen;
  return *this;
}

CStrPart &CStrPart::operator +=(int i)
{
  if (i > m_iLen)
    i = m_iLen;
  m_pBuf += i;
  m_iLen -= i;
  return *this;
}

CStrPart &CStrPart::operator +=(const CStrPart s)
{
  ASSERT(m_iLen >= 0 && s.m_iLen >= 0);
  if (s.EndsAt(0))
    return *this;
  if (EndsAt(0)) {
    *this = s;
    return *this;
  }
  const char *pEnd = Util::Max(m_pBuf + m_iLen, s.m_pBuf + s.m_iLen);
  m_pBuf = Util::Min(m_pBuf, s.m_pBuf);
  m_iLen = (int) (pEnd - m_pBuf);
  return *this;
}

CStrPart CStrPart::operator +(const CStrPart s) const
{
  ASSERT(m_iLen >= 0 && s.m_iLen >= 0);
  if (s.EndsAt(0))
    return *this;
  if (EndsAt(0))
    return s;
  const char *pStart = Util::Min(m_pBuf, s.m_pBuf);
  const char *pEnd = Util::Max(m_pBuf + m_iLen, s.m_pBuf + s.m_iLen);
  return CStrPart(pStart, (int) (pEnd - pStart));
}

// CStrConst ----------------------------------------------------------

CStrConst::THash CStrConst::m_sRepository;

CStrConst::CStrConst(const char *pStr, int iLen)
  : CStr(0, 0)
{
  Init(pStr, iLen);
}

CStrConst::CStrConst(int i)
  : CStr(0, 0)
{
  static char buf[96];
  _itoa(i, buf, 10);
  int iLen = (int) strlen(buf);
  Init(buf, iLen);
}

CStrConst::~CStrConst()
{
  const THeader *pHeader = GetHeader();
  if (pHeader)
    pHeader->Release();
  m_pBuf = 0;
}

void CStrConst::Construct(const CStrBase &s)
{
  if (s.EndsAt(0))
    return;
  THeader *pHeader = const_cast<THeader*>(s.GetHeader());
  if (pHeader) {
    if (!pHeader->m_bInRepository) {
      THash::TIter it = m_sRepository.Find(pHeader);
      if (it)
        pHeader = *it;
      else {
        pHeader->m_bInRepository = true;
        m_sRepository.Add(pHeader);
      }
    }
    m_pBuf = (const char *) (pHeader + 1);
    pHeader->Acquire();
  } else 
    Init(s.m_pBuf, s.Length());
}

CStrConst &CStrConst::operator =(const CStrBase &s)
{
  const THeader *pHeader = GetHeader();
  THeader *pHeaderS = const_cast<THeader *>(s.GetHeader());
  if (pHeaderS) {
    if (!pHeaderS->m_bInRepository) {
      THash::TIter it = m_sRepository.Find(pHeaderS);
      if (it) 
        pHeaderS = *it;
      else {
        pHeaderS->m_bInRepository = true;
        m_sRepository.Add(pHeaderS);
      }
    }
    pHeaderS->Acquire();
    m_pBuf = (const char *) (pHeaderS + 1);
  } else
    Init(s.m_pBuf, s.Length());
  if (pHeader)
    pHeader->Release();
  return *this;
}

void CStrConst::Init(const char *pStr, int iLen)
{
  if (!pStr) {
    m_pBuf = 0;
    return;
  }
  CStrConst::THash::TIter it = m_sRepository.Find(CStrPart(pStr, iLen));
  THeader *pHeader;
  if (it) {
    pHeader = *it;
    ASSERT(pHeader->m_bInRepository);
    m_pBuf = (const char *) (pHeader + 1);
    pHeader->Acquire();
  } else {
    CStr::Init(pStr, iLen);
    pHeader = const_cast<THeader *>(GetHeader());
    pHeader->m_bInRepository = true;
    m_sRepository.Add(pHeader);
  }
}

bool CStrConst::CheckRepository()
{
  bool bRes;
  THash::TIter it;

  bRes = true;
  for (it = m_sRepository; it; ++it) {
    ASSERT(it->m_bInRepository && it->m_bHashInitialized);
    bRes &= it->m_bInRepository && it->m_bHashInitialized;
  }
  return bRes;
}