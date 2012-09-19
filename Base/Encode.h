#ifndef __ENCODE_H
#define __ENCODE_H

class CRangeEncoder {
public:
  static const int MAXCHARS = 257;

  UINT         m_uiCharCounts[MAXCHARS];
  UINT         m_uiTotalCount;
  BYTE         m_btCountShift;
  CArray<BYTE> m_arrOutput;
  DWORD        m_dwIntervalStart, m_dwIntervalSize;
  DWORD        m_dwValue;

  void InitCounts();
  UINT GetCharCount(int iChar);
  void IncCharCount(int iChar);
  UINT CalcTotalCount();
  float CalcEntropy();
  
  void GetCountInterval(int iChar, UINT &uiStart, UINT &uiSize);
  void GetSubInterval(UINT uiStart, UINT uiSize, DWORD &dwIntervalStart, DWORD &dwIntervalSize);
  int  GetIntervalPrefix();
  void AdjustIntervalBoundary();
  void OutputIntervalPrefix();
  void FlushInterval();
  void EncodeChar(int iChar);

  int  FindCharForValue(DWORD &dwSubStart, DWORD &dwSubSize);
  BYTE GetNextEncoded(BYTE *&pEncoded, BYTE *pEncodedEnd); 
  void InitValue(BYTE *&pEncoded, BYTE *pEncodedEnd);  
  void InputIntervalPrefix(BYTE *&pEncoded, BYTE *pEncodedEnd);
  
  void Encode(BYTE *pInput, UINT uiInputSize);
  void Decode(BYTE *pEncoded, UINT uiEncodedSize);
};

#endif