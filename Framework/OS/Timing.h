#ifndef __TIMING_H
#define __TIMING_H

class CTime {
public:
  static const uint64_t FREQUENCY = 1000000000ull; // Default resolution 1ns

  uint64_t m_qwTime;

  explicit CTime(uint64_t qwTime = 0) { m_qwTime = qwTime; }

  CTime &operator =(CTime kTime)      { m_qwTime = kTime.m_qwTime; return *this; }

  bool operator <(CTime kTime)  const { return m_qwTime < kTime.m_qwTime; }
  bool operator <=(CTime kTime) const { return m_qwTime <= kTime.m_qwTime; }
  bool operator >(CTime kTime)  const { return m_qwTime > kTime.m_qwTime; }
  bool operator >=(CTime kTime) const { return m_qwTime >= kTime.m_qwTime; }
  bool operator ==(CTime kTime) const { return m_qwTime == kTime.m_qwTime; }

  CTime operator +(CTime kTime) const { return CTime(m_qwTime + kTime.m_qwTime); }
  CTime operator -(CTime kTime) const { return CTime(m_qwTime - kTime.m_qwTime); }

  float Seconds() const { return m_qwTime / FREQUENCY + (m_qwTime % FREQUENCY) / (float) FREQUENCY; }
};

class CTimerBase: public CObject {
  DEFRTTI(CTimerBase, CObject, false)
public:
  uint64_t m_qwStartTime, m_qwNumerator, m_qwDenominator;

  CTimerBase();

  void Init(float fRatioToReal = 1.0f, bool bKeepCurrentTime = true);

  float GetRatioToReal() const;
  CTime GetCurrent() const;

public:
  uint64_t GetScaledCounter(uint64_t qwCounter) const;

  virtual uint64_t GetTimerFrequency() const = 0;
  virtual uint64_t GetTimerCounter() const = 0;
};

#include "Windows/WinTiming.h"
#include "Linux/LinTiming.h"

#endif
