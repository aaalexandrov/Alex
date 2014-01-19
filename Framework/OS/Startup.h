#ifndef __STARTUP_H
#define __STARTUP_H

#include "RTTI.h"

class CStartUp: public CObject {
  DEFRTTI(CStartUp, CObject, false)
public:
  CStrAny m_sCmdLine;

  CStartUp()          {}
  virtual ~CStartUp() {}
};

int Main(CStartUp &kStartUp);

#endif