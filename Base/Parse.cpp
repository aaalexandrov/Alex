#include "stdafx.h"
#include "Parse.h"
#include <ctype.h>

const static char g_chNumerals[] = "0123456789abcdefghijklmnopqrstuvwxyz";
const static CStrPart g_sWhitespace(" \n\r\t");


CStrPart Parse::ReadChar(CStrPart &s, char ch, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && s.m_pBuf[i] == ch)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadIChar(CStrPart &s, char ch, int iNumber)
{
  int i = 0;
  ch = tolower(ch);
  while (i < iNumber && !s.EndsAt(i) && tolower(s.m_pBuf[i]) == ch)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadChars(CStrPart &s, const CStrBase &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.Find(s.m_pBuf[i]) >= 0)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadIChars(CStrPart &s, const CStrBase &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.IFind(s.m_pBuf[i]) >= 0)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadUntilChar(CStrPart &s, char ch, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && s.m_pBuf[i] != ch)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadUntilIChar(CStrPart &s, char ch, int iNumber)
{
  int i = 0;
  ch = tolower(ch);
  while (i < iNumber && !s.EndsAt(i) && tolower(s.m_pBuf[i]) != ch)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadUntilChars(CStrPart &s, const CStrBase &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.Find(s.m_pBuf[i]) < 0)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadUntilIChars(CStrPart &s, const CStrBase &sChars, int iNumber)
{
  int i = 0;
  while (i < iNumber && !s.EndsAt(i) && sChars.IFind(s.m_pBuf[i]) < 0)
    i++;
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadNewLine(CStrPart &s)
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
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadToNewLine(CStrPart &s)
{
  return ReadUntilChars(s, CStrPart("\r\n", 2));
}

CStrPart Parse::ReadLine(CStrPart &s)
{
  CStrPart sLine = ReadToNewLine(s);
  sLine += ReadNewLine(s);
  return sLine;
}

CStrPart Parse::ReadInt(CStrPart &s, int iRadix)
{
  ASSERT(iRadix <= 36);
  CStrPart sNumerals(g_chNumerals, iRadix);
  return ReadIChars(s, sNumerals);
}

CStrPart Parse::ReadFloat(CStrPart &s, int iRadix)
{
  ASSERT(iRadix <= 36);
  CStrPart sNumerals(g_chNumerals, iRadix);
  CStrPart sInt = ReadIChars(s, sNumerals);
  CStrPart sDot = ReadChar(s, '.', 1);
  if (!sDot)
    return sInt;
  CStrPart sFrac = ReadIChars(s, sNumerals);
  return sInt + sDot + sFrac;
}

CStrPart Parse::ReadIdentifier(CStrPart &s)
{
  int i = 0;
  if (!s.EndsAt(i)) {
    if (isalpha(s.m_pBuf[i]) || s.m_pBuf[i] == '_') {
      i++;
      while (!s.EndsAt(i) && (isalnum(s.m_pBuf[i]) || s.m_pBuf[i] == '_'))
        i++;
    }
  }
  CStrPart sRes(s.m_pBuf, i);
  s += i;
  return sRes;
}

CStrPart Parse::ReadWhitespace(CStrPart &s)
{
  return ReadChars(s, g_sWhitespace);
}

CStrPart Parse::ReadUntilWhitespace(CStrPart &s)
{
  return ReadUntilChars(s, g_sWhitespace);
}

CStrPart Parse::TrimWhitespace(const CStrBase &s)
{
  CStrPart sRes(s, s.Length());
  Parse::ReadWhitespace(sRes);
  return Parse::ReadUntilWhitespace(sRes);
}

CStrPart Parse::ReadStr(CStrPart &s, const CStrBase &sStr)
{
  int iLen = sStr.Length();
  CStrPart sStart = s.SubStr(0, iLen);
  if (sStart == sStr) {
    s += iLen;
    return sStart;
  }
  return CStrPart(0);
}

CStrPart Parse::ReadIStr(CStrPart &s, const CStrBase &sStr)
{
  int iLen = sStr.Length();
  CStrPart sStart = s.SubStr(0, iLen);
  if (!sStart.ICmp(sStr)) {
    s += iLen;
    return sStart;
  }
  return CStrPart(0);
}

CStrPart Parse::MatchDelimiters(CStrPart &s, TDelimiterBlock *pDelimiters, int iDelimiters, int *pIndexFound)
{
  int i, iFound;
  CStrPart sRes, sEnd;

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
  while (s) {
    sEnd = ReadStr(s, pDelimiters[*pIndexFound].sEnd);
    if (!!sEnd) {
      sRes += sEnd;
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
    s += 1;
  }
  s += sRes;
  return CStrPart();
}
