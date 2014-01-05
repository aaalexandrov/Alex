#include "stdafx.h"
#include "Timing.h"

// CTimerBase -----------------------------------------------------------------

CRTTIRegisterer<CTimerBase> g_RegTimerBase;

CTimerBase::CTimerBase()
{
  m_qwDenominator = 0;
}

void CTimerBase::Init(float fRatioToReal, bool bKeepCurrentTime)
{
  if (!m_qwDenominator) {
    m_qwDenominator = GetTimerFrequency();
    bKeepCurrentTime = false;
  }
  uint64_t qwCounter = GetTimerCounter();
  uint64_t qwInitialTime = bKeepCurrentTime ? GetScaledCounter(qwCounter) - m_qwStartTime : 0;
  m_qwNumerator = (uint64_t) (fRatioToReal * (double) CTime::FREQUENCY);
  // Set the starting offset so that the current time is the same as qwInitialTime
  m_qwStartTime = GetScaledCounter(qwCounter) - qwInitialTime;
}

float CTimerBase::GetRatioToReal() const
{
  return (float) (m_qwNumerator / (double) CTime::FREQUENCY);
}

CTime CTimerBase::GetCurrent() const
{
  uint64_t qwCounter = GetTimerCounter();
  return CTime(GetScaledCounter(qwCounter) - m_qwStartTime);
}

uint64_t CTimerBase::GetScaledCounter(uint64_t qwCounter) const
{
  uint64_t qwRem = qwCounter % m_qwDenominator;
  uint64_t qwTime = (qwCounter / m_qwDenominator) * m_qwNumerator + qwRem * m_qwNumerator / m_qwDenominator;
  return qwTime;
}
