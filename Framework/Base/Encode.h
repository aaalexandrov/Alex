#ifndef __ENCODE_H
#define __ENCODE_H

#include "Array.h"

class CRangeEncoder {
public:
  static const int MAXCHARS = 257;

  UINT            m_uiCharCounts[MAXCHARS];
  UINT            m_uiTotalCount;
  uint8_t         m_btCountShift;
  CArray<uint8_t> m_arrOutput;
  uint32_t        m_dwIntervalStart, m_dwIntervalSize;
  uint32_t        m_dwValue;

  void    InitCounts();
  UINT    GetCharCount(int iChar);
  void    IncCharCount(int iChar);
  UINT    CalcTotalCount();
  float   CalcEntropy();
         
  void    GetCountInterval(int iChar, UINT &uiStart, UINT &uiSize);
  void    GetSubInterval(UINT uiStart, UINT uiSize, uint32_t &dwIntervalStart, uint32_t &dwIntervalSize);
  int     GetIntervalPrefix();
  void    AdjustIntervalBoundary();
  void    OutputIntervalPrefix();
  void    FlushInterval();
  void    EncodeChar(int iChar);
         
  int     FindCharForValue(uint32_t &dwSubStart, uint32_t &dwSubSize);
  uint8_t GetNextEncoded(uint8_t *&pEncoded, uint8_t *pEncodedEnd); 
  void    InitValue(uint8_t *&pEncoded, uint8_t *pEncodedEnd);  
  void    InputIntervalPrefix(uint8_t *&pEncoded, uint8_t *pEncodedEnd);
  
  void    Encode(uint8_t *pInput, UINT uiInputSize);
  void    Decode(uint8_t *pEncoded, UINT uiEncodedSize);
};

#endif