#ifndef __LINTIMING_H
#define __LINTIMING_H

#ifdef LINUX

#include "Timing.h"
#include <time.h>

class CTimerLin: public CTimerBase {
  DEFRTTI(CTimerLin, CTimerBase, true)
public:
  time_t m_kInitSec;

  CTimerLin();

public:
  virtual uint64_t GetTimerFrequency() const;
  virtual uint64_t GetTimerCounter() const;
};

typedef CTimerLin CTimer;

#endif

#endif // __LINTIMING_H
