#include "stdafx.h"
#include "Timing.h"

QWORD CTime::s_qwFrequency = 0, 
      CTime::s_qwFrequencyReal = 0,
      CTime::s_qwStartReal = 0;

float CTime::SecondsSince(CTime kStart, bool bReal)
{
  QWORD qwDuration = m_qwTime - kStart.m_qwTime;
  QWORD qwFreq = bReal ? s_qwFrequencyReal : s_qwFrequency;
  return qwDuration / qwFreq + (qwDuration % qwFreq) / (float) qwFreq;
}

float CTime::Seconds(bool bReal)
{
  QWORD qwFreq = bReal ? s_qwFrequencyReal : s_qwFrequency;
  return m_qwTime / qwFreq + (m_qwTime % qwFreq) / (float) qwFreq;
}

bool CTime::InitTiming(float fRatioToReal)
{
  COMPILE_ASSERT(sizeof(QWORD) == sizeof(LARGE_INTEGER));
  bool bRes = !!QueryPerformanceFrequency((LARGE_INTEGER *) &s_qwFrequencyReal);
  s_qwFrequency = (s_qwFrequencyReal * (QWORD) (fRatioToReal * 1024)) / 1024;
  QueryPerformanceCounter((LARGE_INTEGER *) &s_qwStartReal);
  return bRes;
}

float CTime::GetRatioToReal()
{
  return (s_qwFrequency * 1024 / s_qwFrequencyReal) / 1024.0f;
}

CTime CTime::GetCurrent()
{
  QWORD qwTime;
  QueryPerformanceCounter((LARGE_INTEGER *) &qwTime);
  qwTime = s_qwStartReal + (qwTime - s_qwStartReal) * s_qwFrequency / s_qwFrequencyReal;
  return CTime(qwTime);
}

CTime CTime::GetCurrentReal()
{
  QWORD qwReal;
  QueryPerformanceCounter((LARGE_INTEGER *) &qwReal);
  return CTime(qwReal);
}
