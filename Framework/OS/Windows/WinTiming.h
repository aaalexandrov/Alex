#ifndef __WINTIMING_H
#define __WINTIMING_H

#ifdef WINDOWS

#include "Timing.h"

class CTimerWin: public CTimerBase {
  DEFRTTI(CTimerWin, CTimerBase, true)
public:
  CTimerWin(): CTimerBase() {}

public:
  virtual uint64_t GetTimerFrequency() const;
  virtual uint64_t GetTimerCounter() const;
};

typedef CTimerWin CTimer;

#endif

#endif // __WINTIMING_H
