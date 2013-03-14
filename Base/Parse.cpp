#include "stdafx.h"
#include "Parse.h"
#include <ctype.h>

const static CStrAny g_sWhitespace(ST_WHOLE, " \n\r\t");
const static CStrAny g_sSign(ST_WHOLE, "+-");


CStrAny Parse::ReadChar(CStrAny &s, char ch, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && s.m_pBuf[i] == ch)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadIChar(CStrAny &s, char ch, int iNumber)
{
  int i = 0;
  ch = tolower(ch);
  while (i < iNumber && !s.EndsAt(i) && tolower(s.m_pBuf[i]) == ch)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadChars(CStrAny &s, const CStrAny &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.Find(s.m_pBuf[i]) >= 0)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadIChars(CStrAny &s, const CStrAny &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.IFind(s.m_pBuf[i]) >= 0)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadUntilChar(CStrAny &s, char ch, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && s.m_pBuf[i] != ch)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadUntilIChar(CStrAny &s, char ch, int iNumber)
{
  int i = 0;
  ch = tolower(ch);
  while (i < iNumber && !s.EndsAt(i) && tolower(s.m_pBuf[i]) != ch)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadUntilChars(CStrAny &s, const CStrAny &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.Find(s.m_pBuf[i]) < 0)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadUntilIChars(CStrAny &s, const CStrAny &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.IFind(s.m_pBuf[i]) < 0)
    i++;
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadNewLine(CStrAny &s)
{
  int i = 0;
  if (!s.EndsAt(i)) {
    if (s.m_pBuf[i] == '\n') {
      i++;
      if (!s.EndsAt(i) && s.m_pBuf[i] == '\r')
        i++;
    } else
      if (s.m_pBuf[i] == '\r') {
        i++;
        if (!s.EndsAt(i) && s.m_pBuf[i] == '\n')
          i++;
      }
  }
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadToNewLine(CStrAny &s)
{
  return ReadUntilChars(s, CStrAny(ST_WHOLE, "\r\n"));
}

CStrAny Parse::ReadLine(CStrAny &s)
{
  CStrAny sLine = ReadToNewLine(s);
  sLine >>= ReadNewLine(s);
  return sLine;
}

CStrAny Parse::ReadSign(CStrAny &s)
{
  return ReadChars(s, g_sSign, 1);
}

CStrAny Parse::ReadNumerals(CStrAny &s, int iRadix, int iNumber)
{
  ASSERT(iRadix <= 36);
  int i = 0;
  while (i < iNumber && !s.EndsAt(i)) {
    int iVal = Numeral2Value(s.m_pBuf[i]);
    if (iVal < 0 || iVal >= iRadix)
      break;
    i++;
  }
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadInt(CStrAny &s, int iRadix, bool bSign)
{
  CStrAny sRes;
  if (bSign)
    sRes = ReadChars(s, g_sSign, 1);
  sRes >>= ReadNumerals(s, iRadix);
  return sRes;
}

CStrAny Parse::ReadFloat(CStrAny &s, int iRadix, bool bSign)
{
  ASSERT(iRadix <= 36);
  CStrAny sRes = ReadInt(s, iRadix, bSign);
  sRes >>= ReadChar(s, '.', 1);
  sRes >>= ReadNumerals(s, iRadix);
  return sRes;
}

CStrAny Parse::ReadIdentifier(CStrAny &s)
{
  int i = 0;
  if (!s.EndsAt(i)) {
    if (isalpha(s.m_pBuf[i]) || s.m_pBuf[i] == '_') {
      i++;
      while (!s.EndsAt(i) && (isalnum(s.m_pBuf[i]) || s.m_pBuf[i] == '_'))
        i++;
    }
  }
  CStrAny sRes = s.SubStr(0, i);
  s >>= i;
  return sRes;
}

CStrAny Parse::ReadWhitespace(CStrAny &s)
{
  return ReadChars(s, g_sWhitespace);
}

CStrAny Parse::ReadUntilWhitespace(CStrAny &s)
{
  return ReadUntilChars(s, g_sWhitespace);
}

CStrAny Parse::TrimWhitespace(const CStrAny &s)
{
  CStrAny sRes(s, ST_PART);
  Parse::ReadWhitespace(sRes);
  return Parse::ReadUntilWhitespace(sRes);
}

CStrAny Parse::ReadStr(CStrAny &s, const CStrAny &sStr)
{
  int iLen = sStr.Length();
  CStrAny sStart = s.SubStr(0, iLen);
  if (sStart == sStr) {
    s >>= iLen;
    return sStart;
  }
  return CStrAny();
}

CStrAny Parse::ReadIStr(CStrAny &s, const CStrAny &sStr)
{
  int iLen = sStr.Length();
  CStrAny sStart = s.SubStr(0, iLen);
  if (!sStart.ICmp(sStr)) {
    s >>= iLen;
    return sStart;
  }
  return CStrAny();
}

CStrAny Parse::MatchDelimiters(CStrAny &s, TDelimiterBlock *pDelimiters, int iDelimiters, int *pIndexFound)
{
  int i, iFound;
  CStrAny sRes, sEnd;

  if (!iDelimiters || !pDelimiters)
    return sRes;
  if (!pIndexFound)
    pIndexFound = &iFound;
  for (i = 0; i < iDelimiters; i++) {
    sRes = ReadStr(s, pDelimiters[i].sStart);
    if (!!sRes)
      break;
  }
  if (!sRes) {
    *pIndexFound = -1;
    return sRes;
  }
  *pIndexFound = i;
  while (!!s) {
    sEnd = ReadStr(s, pDelimiters[*pIndexFound].sEnd);
    if (!!sEnd) {
      sRes >>= sEnd;
      return sRes;
    }
    if (pDelimiters[*pIndexFound].bRecursive) {
      int iInd;
      sEnd = MatchDelimiters(s, pDelimiters, iDelimiters, &iInd);
      if (!sEnd) {
        if (iInd >= 0)
          break;
      } else
        continue;
    }
    s >>= 1;
  }
  s >>= sRes;
  return CStrAny();
}

int Parse::Numeral2Value(char ch)
{
  ch = tolower(ch);
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  if (ch >= 'a' && ch <= 'z')
    return ch - 'a' + 10;
  return -1;
}

char Parse::Value2Numeral(int i)
{
  if (i >= 0 && i <= 9)
    return '0' + i;
  if (i >= 10 && i <= 36)
    return 'a' + i - 10;
  return 0;
}

bool Parse::Str2Int(int &iResult, CStrAny const &sInt, int iRadix, bool bSign)
{
  int i, iVal, iSign = 1;

  iResult = 0;
  i = 0;
  if (bSign) {
    if (!sInt.EndsAt(0)) {
      switch (sInt[0]) {
        case '-':
          iSign = -1;
        case '+':
          i++;
      }
    }
  }
  while (!sInt.EndsAt(i)) {
    iVal = Numeral2Value(sInt[i]);
    if (iVal < 0 && iVal >= iRadix)
      return false;
    ASSERT(iResult * iRadix + iVal >= iResult);
    iResult = iResult * iRadix + iVal;
    i++;
  }
  iResult *= iSign;

  return true;
}

bool Parse::Str2Float(float &fResult, CStrAny const &sFloat, int iRadix, bool bSign)
{
  bool bNegative = false;
  int iVal;
  CStrAny s(sFloat, ST_PART);

  fResult = 0;
  if (bSign && sFloat.Length()) {
    switch (sFloat[0]) {
      case '-':
        bNegative = true;
      case '+':
        s >>= 1;
    }
  }
  while (s.Length()) {
    iVal = Numeral2Value(s[0]);
    if (iVal < 0 || iVal >= iRadix)
      break;
    fResult = fResult * iRadix + iVal;
    s >>= 1;
  }
  if (!s.Length())
    return true;
  if (s[0] != '.')
    return false;
  s >>= 1;

  float fFrac = 1;
  while (s.Length()) {
    iVal = Numeral2Value(s[0]);
    if (iVal < 0 || iVal >= iRadix)
      return false;
    fFrac /= iRadix;
    fResult += iVal * fFrac;
    s >>= 1;
  }
  if (s.Length())
    return false;

  if (bNegative)
    fResult = -fResult;

  return true;
}

char *Parse::Int2Str(int i, char *pBuf, int iBufLen, int iRadix)
{
  int iLen = 0;
  char *pStart, *pEnd;
  if (i < 0)
    pBuf[iLen++] = '-';
  pStart = pBuf + iLen;
  do {
    pBuf[iLen++] = Value2Numeral(abs(i % iRadix));
    i /= iRadix;
  } while (i && iLen < iBufLen - 1);
  pEnd = pBuf + iLen - 1;
  while (pStart < pEnd)
    Util::Swap(*pStart++, *pEnd--);
  pBuf[iLen] = 0;
  return pBuf;
}

char *Parse::Float2Str(float f, char *pBuf, int iBufLen, int iRadix)
{
  float fWhole, fPower, fRem;
  int i, iLen = 0;
  char *pStart, *pEnd;
  if (f < 0) {
    pBuf[iLen++] = '-';
    i = -(int) f;
    fPower = (float) -iRadix;
  } else {
    i = (int) f;
    fPower = (float) iRadix;
  }
  pStart = pBuf + iLen;
  do {
    pBuf[iLen++] = Value2Numeral(i % iRadix);
    i /= iRadix;
  } while (i && iLen < iBufLen - 1);
  pEnd = pBuf + iLen - 1;
  while (pStart < pEnd)
    Util::Swap(*pStart++, *pEnd--);
  fRem = modff(f, &fWhole);
  if (fRem && iLen < iBufLen - 1) {
    pBuf[iLen++] = '.';
    while (fRem && iLen < iBufLen - 1) {
      fRem = modff(f * fPower, &fWhole);
      i = (int) fmod(fWhole, (float) iRadix);
      fPower *= iRadix;
      pBuf[iLen++] = Value2Numeral(i);
    }
  }
  pBuf[iLen] = 0;
  return pBuf;
}
