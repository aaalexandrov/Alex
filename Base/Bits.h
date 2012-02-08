#ifndef __BITS_H
#define __BITS_H

#include "Debug.h"
#include <memory.h>

template <int BITS = 256>
class CBitArray {
public:
  static const int BIT_COUNT = 256;
  static const int ARRAY_SIZE = BIT_COUNT / (sizeof(UINT) * 8) + !!(BIT_COUNT % (sizeof(UINT) * 8));

  UINT m_uiBits[ARRAY_SIZE];

  static int  BitElementIndex(int iBit)      { ASSERT(iBit >= 0 && iBit < BIT_COUNT); return iBit / (sizeof(UINT) * 8); }
  static UINT BitElementMask(int iBit)       { ASSERT(iBit >= 0 && iBit < BIT_COUNT); return 1 << (iBit % (sizeof(UINT) * 8)); }

  inline void SetAll()                       { memset(m_uiBits, -1, sizeof(m_uiBits)); }
  inline void ClearAll()                     { memset(m_uiBits, 0, sizeof(m_uiBits)); }

  inline BYTE GetBit(int iBit) const         { return !!(m_uiBits[BitElementIndex(iBit)] & BitElementMask(iBit)); }

  inline void SetBit(int iBit)               { m_uiBits[BitElementIndex(iBit)] |= BitElementMask(iBit); }
  inline void ClearBit(int iBit)             { m_uiBits[BitElementIndex(iBit)] &= ~BitElementMask(iBit); }
  inline BYTE SetBit(int iBit, BYTE btValue) { if (btValue) SetBit(iBit); else ClearBit(iBit); return !!btValue; }
};

#endif
