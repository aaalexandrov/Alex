#ifndef __PARSE_H
#define __PARSE_H

#include "Str.h"

namespace Parse {
  static const int MAX_CHARS = 0x7fffffff;

  CStrAny ReadChar(CStrAny &s, char ch, int iNumber = MAX_CHARS);
  CStrAny ReadIChar(CStrAny &s, char ch, int iNumber = MAX_CHARS);

  CStrAny ReadChars(CStrAny &s, const CStrAny &sChars, int iNumber = MAX_CHARS);
  CStrAny ReadIChars(CStrAny &s, const CStrAny &sChars, int iNumber = MAX_CHARS);

  CStrAny ReadUntilChar(CStrAny &s, char ch, int iNumber = MAX_CHARS);
  CStrAny ReadUntilIChar(CStrAny &s, char ch, int iNumber = MAX_CHARS);

  CStrAny ReadUntilChars(CStrAny &s, const CStrAny &sChars, int iNumber = MAX_CHARS);
  CStrAny ReadUntilIChars(CStrAny &s, const CStrAny &sChars, int iNumber = MAX_CHARS);

  CStrAny ReadNewLine(CStrAny &s);
  CStrAny ReadToNewLine(CStrAny &s);
  CStrAny ReadLine(CStrAny &s);

  CStrAny ReadSign(CStrAny &s);
  CStrAny ReadNumerals(CStrAny &s, int iRadix = 10, int iNumber = MAX_CHARS);

  CStrAny ReadInt(CStrAny &s, int iRadix = 10, bool bSigned = true);
  CStrAny ReadFloat(CStrAny &s, int iRadix = 10, bool bSigned = true);
  CStrAny ReadIdentifier(CStrAny &s);

  CStrAny ReadWhitespace(CStrAny &s);
  CStrAny ReadUntilWhitespace(CStrAny &s);
  CStrAny TrimWhitespace(const CStrAny &s);

  CStrAny ReadStr(CStrAny &s, const CStrAny &sStr);
  CStrAny ReadIStr(CStrAny &s, const CStrAny &sStr);

  struct TDelimiterBlock {
    CStrAny sStart, sEnd;
    bool bRecursive;
  };

  CStrAny MatchDelimiters(CStrAny &s, TDelimiterBlock *pDelimiters, int iDelimiters, int *pIndexFound = 0);

  int Numeral2Value(char ch);
  char Value2Numeral(int i);
  bool Str2Int(int &iResult, CStrAny const &sInt, int iRadix = 10, bool bSign = true);
  bool Str2Float(float &fResult, CStrAny const &sFloat, int iRadix = 10, bool bSign = true);
  char *Int2Str(int i, char *pBuf, int iBufLen, int iRadix = 10);
  char *Float2Str(float f, char *pBuf, int iBufLen, int iRadix = 10);
};

#endif
