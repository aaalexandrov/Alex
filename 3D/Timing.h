#ifndef __TIMING_H
#define __TIMING_H

class CTime {
public:
  static uint64_t s_qwFrequency, 
                  s_qwFrequencyReal,
                  s_qwStartReal;

  uint64_t m_qwTime;

  CTime(uint64_t qwTime = 0) { m_qwTime = qwTime; } 

  CTime operator +(CTime kTime) { return CTime(m_qwTime + kTime.m_qwTime); }
  CTime operator -(CTime kTime) { return CTime(m_qwTime - kTime.m_qwTime); }

  float SecondsSince(CTime kStart, bool bReal = false);
  float Seconds(bool bReal = false);

  static bool InitTiming(float fRatioToReal = 1.0f);
  static float GetRatioToReal();
  static CTime GetCurrent();
  static CTime GetCurrentReal();
};

#endif