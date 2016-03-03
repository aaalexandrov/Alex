#ifndef __STR_H
#define __STR_H

#include <string.h>
#include "Hash.h"

inline size_t GetHash(const char *pStr, int iLen);

class CStrAny;
class CStrHeader {
public:
  static const int ALLOC_GRANULARITY = 16;

  mutable CRefCount m_RefCount;
  struct {
    int m_iLen: 30;
    int m_bInRepository : 1;
    mutable int m_bHashInitialized : 1;
  };
  mutable size_t m_uiHash;

  inline uint32_t GetRef() const  { return m_RefCount.Get(); }
  inline void     Acquire() const { m_RefCount.Inc(); }
  inline void     Release(TAllocator &kAlloc) const { m_RefCount.Dec(); if (!m_RefCount.Get()) Delete(this);  }

  static CStrHeader *Get(const char *pSrc, int iLen, bool bInRepository);

  static inline int GetMaxLen(int iLen);
  static inline size_t GetAllocSize(int iLen) { return sizeof(CStrHeader) + GetMaxLen(iLen) + 1; }
  static inline CStrHeader *Alloc(int iLen)    { return (CStrHeader *) NEWARR(char, GetAllocSize(iLen)); }
  static inline void Delete(const CStrHeader *pHeader);

  CStrHeader const *GetUnique() const;
  inline CStrHeader *AssureInRepository();

  inline void Init(int iLen, const char *pSrc) { m_iLen = iLen; m_bInRepository = false; m_bHashInitialized = false; if (pSrc) memcpy(this + 1, pSrc, iLen); ((char *) (this + 1))[pSrc ? iLen : 0] = 0; m_RefCount.m_dwCount = 0; }

  inline size_t GetHash() const { if (!m_bHashInitialized) { m_uiHash = ::GetHash((const char *) (this + 1), m_iLen); m_bHashInitialized = true; } return m_uiHash; }

  // Hash functions
  static inline size_t Hash(CStrHeader const *pHeader)  { return pHeader->GetHash(); }
  static inline size_t Hash(char const *pStr)           { return ::GetHash(pStr, (int) strlen(pStr)); }
  static inline size_t Hash(CStrAny const &s);//           { return s.GetHash(); }
	// Equality predicate
  static inline bool Eq(const char *pKey, CStrHeader const *pHeader)            { return !strcmp(pKey, (const char *) (pHeader + 1)); }
  static inline bool Eq(CStrHeader const *pHeader1, CStrHeader const *pHeader2) { if (pHeader1 == pHeader2) return true; if (pHeader1->m_bInRepository && pHeader2->m_bInRepository || pHeader1->m_iLen != pHeader2->m_iLen) return false; return !memcmp(pHeader1 + 1, pHeader2 + 1, pHeader1->m_iLen * sizeof(char)); }
  static inline bool Eq(CStrAny const &s, CStrHeader const *pHeader);//            { CStrHeader *pHeaderS = const_cast<CStrHeader*>(s.GetHeader()); if (pHeaderS) return Eq(pHeaderS, pHeader); if (s.Length() != pHeader->m_iLen) return false; return !memcmp(s.m_pBuf, (pHeader + 1), pHeader->m_iLen * sizeof(char)); }

public:
  typedef CHash<CStrHeader *, const char *, CStrHeader, CStrHeader> THash;

	static THash &GetRepository() { static THash s_Repository; return s_Repository; }

  static bool CheckRepository();
};

enum EStrType {
  ST_HASHEADER = 1,
  ST_INREPOSITORY = 2,
  ST_ZEROTERMINATED = 4,
  ST_PRESERVETYPE = 16,

  ST_PART = 0,
  ST_WHOLE = ST_ZEROTERMINATED,

  ST_STR = ST_HASHEADER,
  ST_CONST = ST_HASHEADER | ST_INREPOSITORY,
  ST_SAME = ST_PRESERVETYPE,
};

class CStrAny {
public:
  char const *m_pBuf;
  struct {
    int m_iLen: 30;
    int m_bHasHeader: 1;
    int m_bZeroTerminated: 1;
  };

public:
  CStrAny(EStrType eType = ST_STR, char const *pStr = 0, int iLen = -1);
  CStrAny(EStrType eType, int i);
  CStrAny(EStrType eType, float f);
  CStrAny(EStrType eType, char c, int iRepeatCount = 1);
  CStrAny(CStrAny const &s, EStrType eType = ST_SAME);
  explicit CStrAny(CStrHeader const *pHeader);
  ~CStrAny();

  void Init(EStrType eType, char const *pStr, int iLen);
  void Init(EStrType eType, CStrAny const &s);
  void Init(CStrHeader const *pHeader);
  void Done();

  void MakeUnique();
  void AssureHasHeader();
  void AssureInRepository();

  bool operator !() const                  { return EndsAt(0);   }
  bool operator ==(CStrAny const &s) const { return !Cmp(s);     }
	bool operator !=(CStrAny const &s) const { return !!Cmp(s);    }
  bool operator <(CStrAny const &s) const  { return Cmp(s) < 0;  }
  bool operator >(CStrAny const &s) const  { return Cmp(s) > 0;  }
	bool operator <=(CStrAny const &s) const { return Cmp(s) <= 0; }
	bool operator >=(CStrAny const &s) const { return Cmp(s) >= 0; }

  CStrAny &operator =(CStrAny const &s);
  CStrAny &operator =(const char *pStr);

  CStrAny operator +(CStrAny const &s) const;
  CStrAny &operator +=(CStrAny const &s);

  CStrAny operator >>(CStrAny const &s) const;
  CStrAny &operator >>=(CStrAny const &s);

  CStrAny operator >>(int i) const;
  CStrAny &operator >>=(int i);

  void Clear() { Done(); Init(ST_PART, 0, -1); }

  int Length() const { return m_iLen; }
  bool IsEmpty() const { return !m_iLen; }
  bool EndsAt(int iOffset) const { return iOffset >= m_iLen; }

  bool ZeroTerminated() const { return !!m_bZeroTerminated; }
  CStrHeader const *GetHeader() const { return m_bHasHeader ? (CStrHeader const *) m_pBuf - 1 : 0; }
  CStrHeader const *GetHeaderForContents(bool bCreateInRepository) const { return m_bHasHeader ? (CStrHeader const *) m_pBuf - 1 : CStrHeader::Get(m_pBuf, m_iLen, bCreateInRepository); }

  size_t GetHash() const { return m_bHasHeader ? GetHeader()->GetHash() : ::GetHash(m_pBuf, m_iLen); }

  char operator[](int i) const { ASSERT(i >= 0 && i < m_iLen); return m_pBuf[i]; }

  CStrAny SubStr(int iStart, int iEnd, EStrType eType = ST_PART) const;
  int Find(CStrAny const &s, int iStartPos = 0) const;
  int IFind(CStrAny const &s, int iStartPos = 0) const;
  int Find(char ch, int iStartPos = 0) const;
  int IFind(char ch, int iStartPos = 0) const;
  int Cmp(CStrAny const &s) const;
  int ICmp(CStrAny const &s) const;

  inline bool StartsWith(CStrAny const &s) const              { return !SubStr(0, s.Length(), ST_PART).Cmp(s); }
  inline bool EndsWith(CStrAny const &s) const                { return !SubStr(Length() - s.Length(), Length(), ST_PART).Cmp(s); }
  inline bool HasSubStrAt(CStrAny const &s, int iStart) const { return !SubStr(iStart, iStart + s.Length(), ST_PART).Cmp(s); }

  // Hash functions
  static inline size_t Hash(CStrAny const &s) { return s.GetHash(); }
  static inline size_t Hash(const char *pStr) { return ::GetHash(pStr, (int) strlen(pStr)); }

  static inline bool Eq(CStrHeader const *pHeader, CStrAny const &s) { if (s.GetHeader()) return CStrHeader::Eq(pHeader, s.GetHeader()); if (pHeader->m_iLen != s.m_iLen) return false; return !memcmp(pHeader + 1, s.m_pBuf, pHeader->m_iLen); }
	static inline bool Eq(CStrAny const &s0, CStrAny const &s1)        { return s0 == s1; }

protected:
  CStrHeader *GetConcatenationHeader(CStrAny const &s, bool bForceNew) const;
  void GetUnionBeginEnd(CStrAny const &s, char const *&pStart, char const *&pEnd, bool &bZeroTerminated) const;
};

// ------------------------------------------

inline int CStrHeader::GetMaxLen(int iLen)
{
  int iMod = iLen % ALLOC_GRANULARITY;
  if (iMod)
    return iLen + ALLOC_GRANULARITY - iMod;
  return iLen;
}

inline void CStrHeader::Delete(const CStrHeader *pHeader)
{
  if (pHeader->m_bInRepository)
    GetRepository().RemoveValue(const_cast<CStrHeader *>(pHeader));
  DELARR(GetAllocSize(pHeader->m_iLen), (char *) pHeader);
}

inline CStrHeader *CStrHeader::AssureInRepository()
{
  if (m_bInRepository)
    return this;
  THash::TIter it = GetRepository().Find(this);
  if (it)
    return *it;
  m_bInRepository = true;
  GetRepository().Add(this);
  return this;
}

inline size_t GetHash(const char *pStr, int iLen)
{
  unsigned int i;
  size_t uiRes = 0, uiTemp;
  size_t *pBuf = (size_t *) (void *) pStr;
  for (i = 0; i < (iLen * sizeof(char)) / sizeof(size_t); i++)
    uiRes += *pBuf++;
  uiTemp = 0;
  uint8_t *p = (uint8_t *) pBuf;
  switch ((iLen * sizeof(char)) % sizeof(size_t)) {
    case  7: uiTemp |= (uint64_t) (p[6]) << 40;
    case  6: uiTemp |= (uint64_t) (*(uint16_t *) (p + 4)) << 32;
             uiTemp |= *(uint32_t *) p;
             break;
    case  5: uiTemp |= (uint64_t) (p[4]) << 32;
    case  4: uiTemp |= *(uint32_t *) p;
             break;
    case  3: uiTemp |= (uint32_t) (p[2]) << 16;
    case  2: uiTemp |= *(uint16_t *) p;
             break;
    case  1: uiTemp |= *p;
    case  0:
             break;
    default: ASSERT(!"size_t is more than 8 bytes, modify the trailing chars addition code");
             break;
  }
  return uiRes + uiTemp;
}

inline size_t CStrHeader::Hash(CStrAny const &s)                        { return s.GetHash(); }
inline bool CStrHeader::Eq(CStrAny const &s, CStrHeader const *pHeader) { CStrHeader *pHeaderS = const_cast<CStrHeader*>(s.GetHeader()); if (pHeaderS) return Eq(pHeaderS, pHeader); if (s.Length() != pHeader->m_iLen) return false; return !memcmp(s.m_pBuf, (pHeader + 1), pHeader->m_iLen * sizeof(char)); }


#endif
