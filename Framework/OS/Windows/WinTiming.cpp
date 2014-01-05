#include "stdafx.h"
#include "WinTiming.h"

#ifdef WINDOWS

#include <Windows.h>

COMPILE_ASSERT(sizeof(uint64_t) == sizeof(LARGE_INTEGER));

// CTimerWin ------------------------------------------------------------------

CRTTIRegisterer<CTimerWin> g_RegTimerWin;

uint64_t CTimerWin::GetTimerFrequency() const
{
  LARGE_INTEGER kFreq;
  QueryPerformanceFrequency(&kFreq);
  return kFreq.QuadPart;
}

uint64_t CTimerWin::GetTimerCounter() const
{
  LARGE_INTEGER kCounter;
  QueryPerformanceCounter(&kCounter);
  return kCounter.QuadPart;
}

#endif
