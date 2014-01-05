#include "stdafx.h"
#include "Debug.h"
#include <stdarg.h>
#include <stdio.h>
#ifdef WINDOWS
    #include <Windows.h>
#endif

bool g_bAssertDisabled = false;
bool g_bInAssert = false;

void DbgPrint(const char *pFormat, ...)
{
  va_list argptr;
  va_start(argptr, pFormat);

#ifdef _MSC_VER
  char chBuf[2048];
  vsprintf(chBuf, pFormat, argptr);
  OutputDebugStringA(chBuf);
#else // __GNUC__
  vfprintf(stderr, pFormat, argptr);
#endif
}

void Assert(bool bCondition, const char *pCondition, const char *pFile, int nLine)
{
  int iRes;
  char chBuf[4096];

  if (bCondition || g_bAssertDisabled || g_bInAssert)
    return;

  g_bInAssert = true;

  DbgPrint("ASSERT(%s) at %s:%d\n", pCondition, pFile, nLine);

#ifdef WINDOWS
  sprintf(chBuf, "ASSERT(%s)\n\nat %s:%d\n\nBreak execution (Y/N) or disable assertions (Cancel)?", pCondition, pFile, nLine);
  iRes = MessageBoxA(0, chBuf, "Assertion failed!", MB_YESNOCANCEL | MB_ICONSTOP);
  switch (iRes) {
    case IDCANCEL:
      g_bAssertDisabled = true;
      break;
    case IDYES:
      DebugBreak();
      break;
    case IDNO:
      break;
  }
#else
  __builtin_trap();
#endif

  g_bInAssert = false;
}
