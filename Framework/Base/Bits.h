#ifndef __BITS_H
#define __BITS_H

#include "Debug.h"
#include "Mem.h"
#include <memory.h>

template <int BITS>
class CBitsStatic {
public:
  static const int BIT_COUNT = BITS;
  static const int ARRAY_SIZE = BIT_COUNT / (sizeof(UINT) * 8) + !!(BIT_COUNT % (sizeof(UINT) * 8));

  UINT m_uiBits[ARRAY_SIZE];

  CBitsStatic(TAllocator &kAllocator)         {}

  inline UINT *GetBits()                      { return m_uiBits; }
  inline UINT const *GetBits() const          { return m_uiBits; }
  inline int  GetBitCount() const             { return BIT_COUNT; }
  inline int  GetArraySize() const            { return ARRAY_SIZE; }

  inline void SetBitCount(int iCount)         { ASSERT(iCount == BIT_COUNT || !"Can't resize static bit array"); }
};

class CBitsDynamic {
public:
  TAllocator *m_pAllocator;
  UINT       *m_pBits;
  int         m_iBitCount;

  CBitsDynamic(TAllocator &kAllocator): m_pAllocator(&kAllocator), m_pBits(0), m_iBitCount(0)  {}
  ~CBitsDynamic()                             { DELARR_A(*m_pAllocator, GetArraySize(), m_pBits); }

  inline UINT *GetBits()                      { return m_pBits; }
  inline UINT const *GetBits() const          { return m_pBits; }
  inline int  GetBitCount() const             { return m_iBitCount; }
  inline int  GetArraySize() const            { return GetArraySize(m_iBitCount); }

  static inline int GetArraySize(int iBits)   { return iBits / (sizeof(UINT) * 8) + !!(iBits % (sizeof(UINT) * 8)); }

  inline void SetBitCount(int iBitCount);
};

template <class B>
class CBitArrayTpl {
public:
  B m_Bits;

  CBitArrayTpl(TAllocator &kAllocator): m_Bits(kAllocator)   {}

  inline int  BitElementIndex(int iBit) const       { ASSERT(iBit >= 0 && iBit < m_Bits.GetBitCount()); return iBit / (sizeof(UINT) * 8); }
  inline UINT BitElementMask(int iBit)  const       { ASSERT(iBit >= 0 && iBit < m_Bits.GetBitCount()); return 1 << (iBit % (sizeof(UINT) * 8)); }

  inline void SetAll()                              { memset(m_Bits.GetBits(), -1, m_Bits.GetArraySize() * sizeof(UINT)); }
  inline void ClearAll()                            { memset(m_Bits.GetBits(), 0, m_Bits.GetArraySize() * sizeof(UINT)); }

  inline uint8_t GetBit(int iBit) const             { return !!(m_Bits.GetBits()[BitElementIndex(iBit)] & BitElementMask(iBit)); }

  inline void SetBit(int iBit)                      { m_Bits.GetBits()[BitElementIndex(iBit)] |= BitElementMask(iBit); }
  inline void ClearBit(int iBit)                    { m_Bits.GetBits()[BitElementIndex(iBit)] &= ~BitElementMask(iBit); }
  inline uint8_t SetBit(int iBit, uint8_t btValue)  { if (btValue) SetBit(iBit); else ClearBit(iBit); return !!btValue; }

	inline int ScanFirstSet(int iStartBit = 0) const;
	inline int ScanFirstClear(int iStartBit = 0) const;
};

template <int BITS = 256>
class CBitArray: public CBitArrayTpl<CBitsStatic<BITS> > {};

class CBitDynArray: public CBitArrayTpl<CBitsDynamic> {
public:
  CBitDynArray(int iBitCount = 256, TAllocator &kAllocator = DEF_ALLOC): CBitArrayTpl<CBitsDynamic>(kAllocator) { this->m_Bits.SetBitCount(iBitCount); }
};

// Implementation -------------------------------------------------------------

void CBitsDynamic::SetBitCount(int iBitCount)
{
  int iCurSize = GetArraySize(m_iBitCount);
  int iNewSize = GetArraySize(iBitCount);

  if (iCurSize != iNewSize) {
    UINT *pCurBits = m_pBits;
    if (iBitCount) {
      m_pBits = NEWARR_A(*m_pAllocator, UINT, iNewSize);
      if (pCurBits)
        memcpy(m_pBits, pCurBits, Util::Min(iNewSize, iCurSize) * sizeof(UINT));
    } else
			m_pBits = 0;
    if (pCurBits)
      DELARR_A(*m_pAllocator, iCurSize, pCurBits);
  }
  m_iBitCount = iBitCount;
}

template <class B>
int CBitArrayTpl<B>::ScanFirstSet(int iStartBit) const
{
	if (!m_Bits.GetArraySize())
		return -1;
	for (int i = BitElementIndex(iStartBit); i < m_Bits.GetArraySize(); ++i)
		if (m_Bits.GetBits()[i]) {
			int iStart = Util::Max<int>(i * sizeof(UINT) * 8, iStartBit);
			int iEnd = Util::Min<int>(iStart + sizeof(UINT) * 8, m_Bits.GetBitCount());
			for (int iBit = iStart; iBit < iEnd; ++iBit)
				if (GetBit(iBit))
					return iBit;
		}
	return -1;
}

template <class B>
int CBitArrayTpl<B>::ScanFirstClear(int iStartBit) const
{
	if (!m_Bits.GetArraySize())
		return -1;
	for (int i = BitElementIndex(iStartBit); i < m_Bits.GetArraySize(); ++i)
		if (m_Bits.GetBits()[i] != (UINT) -1) {
			int iStart = Util::Max<int>(i * sizeof(UINT) * 8, iStartBit);
			int iEnd = Util::Min<int>(iStart + sizeof(UINT) * 8, m_Bits.GetBitCount());
			for (int iBit = iStart; iBit < iEnd; ++iBit)
				if (!GetBit(iBit))
					return iBit;
		}
	return -1;
}

#endif
