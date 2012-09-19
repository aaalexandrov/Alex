#include "stdafx.h"
#include "Encode.h"
#include <math.h>

void CRangeEncoder::InitCounts()
{
  m_btCountShift = 0;
  for (int i = 0; i < MAXCHARS; ++i)
    m_uiCharCounts[i] = 0;
  m_uiTotalCount = MAXCHARS;
  m_dwIntervalStart = 0;
  m_dwIntervalSize = 0xffffffff;
}

UINT CRangeEncoder::GetCharCount(int iChar)
{
  ASSERT(iChar >= 0 && iChar < MAXCHARS);
  UINT uiCount = m_uiCharCounts[iChar] >> m_btCountShift;
  return uiCount ? uiCount : 1;
}

void CRangeEncoder::IncCharCount(int iChar)
{
  m_uiCharCounts[iChar]++;
  UINT uiMask = 1 << m_btCountShift;
  if (m_uiCharCounts[iChar] > uiMask && !(m_uiCharCounts[iChar] & (uiMask - 1))) {
    m_uiTotalCount++;
    if (m_uiTotalCount > 0xffff) {
      m_btCountShift++;
      m_uiTotalCount = CalcTotalCount();
    }
  }
}

UINT CRangeEncoder::CalcTotalCount()
{
  UINT uiTotalCount = 0;
  for (int i = 0; i < MAXCHARS; ++i)
    uiTotalCount += GetCharCount(i);
  return uiTotalCount;
}

float CRangeEncoder::CalcEntropy()
{
  int i;
  UINT uiTotalCount = 0;
  for (i = 0; i < MAXCHARS; ++i)
    uiTotalCount += m_uiCharCounts[i];
  float fEntropy = 0;
  for (i = 0; i < MAXCHARS; ++i) {
    float fProbability = m_uiCharCounts[i] / (float) uiTotalCount;
    if (fProbability > 0.0f)
      fEntropy -= fProbability * log(fProbability) / log(2.0f);
  }
  return fEntropy;
}

void CRangeEncoder::GetCountInterval(int iChar, UINT &uiStart, UINT &uiSize)
{
  ASSERT(iChar >= 0 && iChar < MAXCHARS);
  uiStart = 0;
  for (int i = 0; i < iChar; ++i)
    uiStart += GetCharCount(i);
  uiSize = GetCharCount(iChar);
}

void CRangeEncoder::GetSubInterval(UINT uiStart, UINT uiSize, DWORD &dwIntervalStart, DWORD &dwIntervalSize)
{
  ASSERT(m_uiTotalCount <= 0xffff);
  dwIntervalStart = m_dwIntervalStart + uiStart * (QWORD) m_dwIntervalSize / m_uiTotalCount;
  dwIntervalSize = uiSize * (QWORD) m_dwIntervalSize / m_uiTotalCount;
  ASSERT(dwIntervalSize);
}

int CRangeEncoder::GetIntervalPrefix()
{
  if ((m_dwIntervalStart >> 24) == ((m_dwIntervalStart + m_dwIntervalSize - 1) >> 24) && (m_dwIntervalSize & 0xffffff))
    return m_dwIntervalStart >> 24;
  return -1;
}

void CRangeEncoder::AdjustIntervalBoundary()
{
  DWORD dwMiddle = (m_dwIntervalStart + m_dwIntervalSize - 1) & 0xff000000;
  ASSERT(m_dwIntervalStart <= dwMiddle && dwMiddle < m_dwIntervalStart + m_dwIntervalSize);
  if (dwMiddle - m_dwIntervalStart >= m_dwIntervalStart + m_dwIntervalSize - dwMiddle)
    m_dwIntervalSize = dwMiddle - m_dwIntervalStart;
  else {
    m_dwIntervalSize -= dwMiddle - m_dwIntervalStart;
    m_dwIntervalStart = dwMiddle;
  }
}

void CRangeEncoder::OutputIntervalPrefix()
{
  int iPrefix;
  while ((iPrefix = GetIntervalPrefix()) >= 0) {
    m_arrOutput.Append(iPrefix);
    m_dwIntervalStart <<= 8;
    m_dwIntervalSize <<= 8;
  }
}

void CRangeEncoder::FlushInterval()
{
  while (m_dwIntervalSize) {
    m_arrOutput.Append(m_dwIntervalStart >> 24);
    m_dwIntervalStart <<= 8;
    m_dwIntervalSize <<= 8;
  }
}

void CRangeEncoder::EncodeChar(int iChar)
{
  UINT uiCountStart, uiCountSize;
  GetCountInterval(iChar, uiCountStart, uiCountSize);
  GetSubInterval(uiCountStart, uiCountSize, m_dwIntervalStart, m_dwIntervalSize);
  
  while (1) {
    OutputIntervalPrefix();
    if (m_dwIntervalSize >= 0xffff) 
      break;
    AdjustIntervalBoundary();
  }

  IncCharCount(iChar);
  ASSERT(m_dwIntervalSize >= 0xffff);
}

void CRangeEncoder::Encode(BYTE *pInput, UINT uiInputSize)
{
  InitCounts();
  m_arrOutput.Clear();

  BYTE *pInputEnd = pInput + uiInputSize;
  while (pInput < pInputEnd)
    EncodeChar(*pInput++);

  EncodeChar(MAXCHARS - 1);
  FlushInterval();
}

int CRangeEncoder::FindCharForValue(DWORD &dwSubStart, DWORD &dwSubSize)
{
  UINT uiCountStart = 0;
  for (int i = 0; i < MAXCHARS; ++i) {
    UINT uiCharCount = GetCharCount(i);
    GetSubInterval(uiCountStart, uiCharCount, dwSubStart, dwSubSize);
    if (m_dwValue >= dwSubStart && m_dwValue < dwSubStart + dwSubSize)
      return i;
    uiCountStart += uiCharCount;
  }
  ASSERT(0);
  return -1;
}

BYTE CRangeEncoder::GetNextEncoded(BYTE *&pEncoded, BYTE *pEncodedEnd)
{
  return pEncoded < pEncodedEnd ? *pEncoded++ : 0;
}

void CRangeEncoder::InitValue(BYTE *&pEncoded, BYTE *pEncodedEnd)
{
  m_dwValue = GetNextEncoded(pEncoded, pEncodedEnd);
  m_dwValue = (m_dwValue << 8) | GetNextEncoded(pEncoded, pEncodedEnd);
  m_dwValue = (m_dwValue << 8) | GetNextEncoded(pEncoded, pEncodedEnd);
  m_dwValue = (m_dwValue << 8) | GetNextEncoded(pEncoded, pEncodedEnd);
}

void CRangeEncoder::InputIntervalPrefix(BYTE *&pEncoded, BYTE *pEncodedEnd)
{
    while (GetIntervalPrefix() >= 0) {
      m_dwIntervalStart <<= 8;
      m_dwIntervalSize <<= 8;
      m_dwValue = (m_dwValue << 8) | GetNextEncoded(pEncoded, pEncodedEnd);
      ASSERT(pEncoded <= pEncodedEnd);
    }
}

void CRangeEncoder::Decode(BYTE *pEncoded, UINT uiEncodedSize)
{
  InitCounts();
  m_arrOutput.Clear();

  BYTE *pEncodedEnd = pEncoded + uiEncodedSize;
  InitValue(pEncoded, pEncodedEnd);

  int iChar;
  DWORD dwSubStart, dwSubSize;
  while ((iChar = FindCharForValue(dwSubStart, dwSubSize)) != MAXCHARS - 1) {
    m_dwIntervalStart = dwSubStart;
    m_dwIntervalSize = dwSubSize;
    m_arrOutput.Append(iChar);

    while (1) {
      InputIntervalPrefix(pEncoded, pEncodedEnd);
      if (m_dwIntervalSize >= 0xffff) 
        break;
      AdjustIntervalBoundary();
    }

    IncCharCount(iChar);
    ASSERT(m_dwIntervalSize >= 0xffff);
  }
  ASSERT(pEncoded == pEncodedEnd);
}
