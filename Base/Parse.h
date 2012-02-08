#ifndef __PARSE_H
#define __PARSE_H

#include "Str.h"

namespace Parse {
  static const int MAX_CHARS = 0x7fffffff;

  CStrPart ReadChar(CStrPart &s, char ch, int iNumber = MAX_CHARS);
  CStrPart ReadIChar(CStrPart &s, char ch, int iNumber = MAX_CHARS);

  CStrPart ReadChars(CStrPart &s, const CStrBase &sChars, int iNumber = MAX_CHARS);
  CStrPart ReadIChars(CStrPart &s, const CStrBase &sChars, int iNumber = MAX_CHARS);

  CStrPart ReadUntilChar(CStrPart &s, char ch, int iNumber = MAX_CHARS);
  CStrPart ReadUntilIChar(CStrPart &s, char ch, int iNumber = MAX_CHARS);

  CStrPart ReadUntilChars(CStrPart &s, const CStrBase &sChars, int iNumber = MAX_CHARS);
  CStrPart ReadUntilIChars(CStrPart &s, const CStrBase &sChars, int iNumber = MAX_CHARS);

  CStrPart ReadNewLine(CStrPart &s);
  CStrPart ReadToNewLine(CStrPart &s);
  CStrPart ReadLine(CStrPart &s);

  CStrPart ReadInt(CStrPart &s, int iRadix = 10);
  CStrPart ReadFloat(CStrPart &s, int iRadix = 10);
  CStrPart ReadIdentifier(CStrPart &s);

  CStrPart ReadWhitespace(CStrPart &s);
  CStrPart ReadUntilWhitespace(CStrPart &s);
  CStrPart TrimWhitespace(const CStrBase &s);

  CStrPart ReadStr(CStrPart &s, const CStrBase &sStr);
  CStrPart ReadIStr(CStrPart &s, const CStrBase &sStr);

  struct TDelimiterBlock {
    CStr sStart, sEnd;
    bool bRecursive;
  };

  CStrPart MatchDelimiters(CStrPart &s, TDelimiterBlock *pDelimiters, int iDelimiters, int *pIndexFound = 0);
};

#endif
