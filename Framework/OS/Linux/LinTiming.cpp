#include "stdafx.h"
#include "LinTiming.h"

#ifdef LINUX

// CTimerLin ------------------------------------------------------------------

CRTTIRegisterer<CTimerLin> g_RegTimerLin;

CTimerLin(): CTimerBase()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  m_kInitSec = ts.tv_sec;
}

uint64_t CTimerLin::GetTimerFrequency() const
{
  return 1000000000ull;
}

uint64_t CTimerLin::GetTimerCounter() const
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  uint64_t qwTime = (ts.tv_sec - m_kInitSec) * 1000000000ull + ts.tv_nsec;
  return qwTime;
}

#endif
