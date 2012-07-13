#ifndef __STR_H
#define __STR_H

#include <string.h>
#include "Hash.h"

inline size_t GetHash(const char *pStr, int iLen);

class CStr;
class CStrPart;
class CStrBase {
public:
  class THeader {
  public:
    static const int ALLOC_GRANULARITY = 16;

    mutable CRefCount m_RefCount;        
    struct {
      int m_iLen: 30;
      int m_bInRepository : 1;
      int m_bHashInitialized : 1;
    };
    size_t m_uiHash;

    inline DWORD GetRef() const { return m_RefCount.Get(); } 
    inline void  Acquire() const { m_RefCount.Inc(); }        
    inline void  Release() const { m_RefCount.Dec(); if (!m_RefCount.Get()) Delete(this);  }

    static inline int GetMaxLen(int iLen);
    static inline THeader *Alloc(int iLen)            { return (THeader *) new char[sizeof(THeader) + GetMaxLen(iLen) + 1]; }
    static inline void Delete(const THeader *pHeader);

    inline void Init(int iLen, const char *pSrc) { m_iLen = iLen; m_bInRepository = false; m_bHashInitialized = false; if (pSrc) memcpy(this + 1, pSrc, iLen); ((char *) (this + 1))[pSrc ? iLen : 0] = 0; m_RefCount.m_dwCount = 1; }
    
    inline size_t GetHash() { if (!m_bHashInitialized) { m_uiHash = ::GetHash((const char *) (this + 1), m_iLen); m_bHashInitialized = true; } return m_uiHash; }
  };

public:
  const char *m_pBuf;

  CStrBase()                  {}
  virtual ~CStrBase()         {}

  operator const char *() const             { return m_pBuf;     }
  bool operator !() const                   { return EndsAt(0);  }
  bool operator ==(const CStrBase &s) const { return !Cmp(s);    }
  bool operator <(const CStrBase &s) const  { return Cmp(s) < 0; }
  bool operator >(const CStrBase &s) const  { return Cmp(s) > 0; }

  virtual int Length() const = 0;
  virtual bool EndsAt(int iOffset) const = 0;

  virtual bool ZeroTerminated() const = 0;
  virtual const THeader *GetHeader() const = 0;

  virtual CStrBase &Assign(const CStrBase &s) = 0;

  virtual size_t GetHash() const = 0;

  char operator[](int i) const { return m_pBuf[i]; }

  CStrPart SubStr(int iStart, int iEnd) const;
  int Find(const CStrBase &s, int iStartPos = 0) const;
  int IFind(const CStrBase &s, int iStartPos = 0) const;
  int Find(char ch, int iStartPos = 0) const;
  int IFind(char ch, int iStartPos = 0) const;
  int Cmp(const CStrBase &s) const;
  int ICmp(const CStrBase &s) const;

  inline bool StartsWith(const CStrBase &s) const;
  inline bool EndsWith(const CStrBase &s) const;
  inline bool HasSubStrAt(const CStrBase &s, int iStart) const;

  // Hash functions
  static inline size_t Hash(const CStrBase &s) { return s.GetHash(); }
  static inline size_t Hash(CStrBase::THeader *pHeader)  { return pHeader->GetHash(); }
  static inline size_t Hash(const char *pStr) { return ::GetHash(pStr, (int) strlen(pStr)); }
	// Equality predicate
  static inline bool Eq(const char *pKey, CStrBase::THeader *pHeader) { return !strcmp(pKey, (const char *) (pHeader + 1)); }
  static inline bool Eq(CStrBase const &s, CStrBase::THeader *pHeader) { THeader *pHeaderS = const_cast<THeader*>(s.GetHeader()); if (pHeaderS) return Eq(pHeaderS, pHeader); if (s.Length() != pHeader->m_iLen) return false; return !memcmp(s.m_pBuf, (pHeader + 1), pHeader->m_iLen * sizeof(char)); }
  static inline bool Eq(CStrBase::THeader *pHeader1, CStrBase::THeader *pHeader2) { if (pHeader1 == pHeader2) return true; if (pHeader1->m_iLen != pHeader2->m_iLen) return false; return !memcmp(pHeader1 + 1, pHeader2 + 1, pHeader1->m_iLen * sizeof(char)); }
};

class CStr: public CStrBase {
public:
  CStr(const char *pStr = 0, int iLen = -1);
  CStr(int i);
  CStr(char c, int iRepeatCount = 1);
  CStr(const CStrBase &s) { Construct(s); }
  CStr(const CStr &s)     { Construct(s); }
  virtual ~CStr();

  void Construct(const CStrBase &s);

  virtual CStr &operator =(const CStrBase &s);
  CStr &operator =(const char *pStr);

  CStr &operator =(const CStr &s) { return operator =(*(CStrBase *) &s); }

  virtual CStrBase &Assign(const CStrBase &s) { return *this = s; }

  CStr operator +(const CStrBase &s) const;
  CStr &operator +=(const CStrBase &s);

  virtual int Length() const               { const THeader *pHeader = GetHeader(); return pHeader ? pHeader->m_iLen : 0; }
  virtual bool EndsAt(int iOffset) const   { return !m_pBuf || !m_pBuf[iOffset];          }

  virtual bool ZeroTerminated() const      { return true;                                 }
  virtual const THeader *GetHeader() const { return m_pBuf ? (const THeader *) m_pBuf - 1 : 0; }

  virtual size_t GetHash() const           { THeader *pHeader = (THeader *) GetHeader(); return pHeader ? pHeader->GetHash() : 0; }

  virtual void Init(const char *pStr, int iLen = -1);
  void MakeUnique();

  CStr &ToUpper();
  CStr &ToLower();
};

class CStrPart: public CStrBase {
public:
  int m_iLen;

  explicit CStrPart(const char *pStr = 0, int iLen = -1);
           CStrPart(const CStrPart &s) { m_pBuf = s.m_pBuf; m_iLen = s.m_iLen; }
  explicit CStrPart(const CStrBase &s) { m_pBuf = s.m_pBuf; m_iLen = s.Length(); }

  CStrPart &operator =(const CStrBase &s) { m_pBuf = s.m_pBuf; m_iLen = s.Length(); return *this; }
//  CStrPart &operator =(const char *pStr)  { return Set(pStr);                                     }

  CStrPart &Set(const char *pStr, int iLen = -1);

  virtual CStrBase &Assign(const CStrBase &s) { ASSERT(!"CStrPart cannot be assigned to!"); return *this; }

  virtual size_t GetHash() const { return ::GetHash(m_pBuf, m_iLen); }

  CStrPart &operator +=(int i);
  CStrPart &operator +=(const CStrPart s);
  CStrPart operator +(const CStrPart s) const;

  virtual int Length() const              { return m_iLen;            }
  virtual bool EndsAt(int iOffset) const  { return iOffset >= m_iLen; }

  virtual bool ZeroTerminated() const     { return false;             }
  virtual const THeader *GetHeader() const { return 0; }
};

class CStrConst: public CStr {
public:
	CStrConst(const char *pStr = 0, int iLen = -1);
	CStrConst(int i);
  CStrConst(const CStrBase &s)  : CStr((const char *) 0, 0) { Construct(s); }
  CStrConst(const CStrConst &s) : CStr((const char *) 0, 0) { Construct(s); }

  virtual ~CStrConst();

  void Construct(const CStrBase &s);

  virtual CStrConst &operator =(const CStrBase &s);
  CStrConst &operator =(const CStrConst &s) { return operator =((const CStrBase &) s); }

  virtual void Init(const char *pStr, int iLen = -1);
public:
  typedef CHash<CStrBase::THeader *, const char *, CStrConst, CStrConst> THash;

	static THash m_sRepository;

  static bool CheckRepository();
};

// ------------------------------------------

inline int CStrBase::THeader::GetMaxLen(int iLen)             
{ 
  int iMod = iLen % ALLOC_GRANULARITY;
  if (iMod)
    return iLen + ALLOC_GRANULARITY - iMod; 
  return iLen;
} 

inline void CStrBase::THeader::Delete(const THeader *pHeader)
{
  if (pHeader->m_bInRepository) 
    CStrConst::m_sRepository.RemoveValue(const_cast<THeader *>(pHeader)); 
  delete [] (char *) pHeader; 
}

inline bool CStrBase::StartsWith(const CStrBase &s) const              { return !SubStr(0, s.Length()).Cmp(s); }
inline bool CStrBase::EndsWith(const CStrBase &s) const                { return !SubStr(Length() - s.Length(), Length()).Cmp(s); }
inline bool CStrBase::HasSubStrAt(const CStrBase &s, int iStart) const { return !SubStr(iStart, iStart + s.Length()).Cmp(s); }

inline size_t GetHash(const char *pStr, int iLen)
{
  unsigned int i;
  size_t uiRes = 0, uiTemp;
  size_t *pBuf = (size_t *) (void *) pStr;
  for (i = 0; i < (iLen * sizeof(char)) / sizeof(size_t); i++)
    uiRes += *pBuf++;
  uiTemp = 0;
  BYTE *p = (BYTE *) pBuf;
  switch ((iLen * sizeof(char)) % sizeof(size_t)) {
    case  7: uiTemp |= (QWORD) (p[6]) << 40;
    case  6: uiTemp |= (QWORD) (*(WORD *) (p + 4)) << 32;
             uiTemp |= *(DWORD *) p;
             break;
    case  5: uiTemp |= (QWORD) (p[4]) << 32;
    case  4: uiTemp |= *(DWORD *) p;
             break;
    case  3: uiTemp |= (DWORD) (p[2]) << 16;
    case  2: uiTemp |= *(WORD *) p;
             break;
    case  1: uiTemp |= *p;
    case  0:
             break;
    default: ASSERT(!"size_t is more than 8 bytes, modify the trailing chars addition code");
             break;
  }
  return uiRes + uiTemp;
}

#endif
