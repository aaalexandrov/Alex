#ifndef __STARTUP_H
#define __STARTUP_H

#include "../Base/RTTI.h"
#include "../Base/Str.h"

class CStartUp: public CObject {
  DEFRTTI(CStartUp, CObject, false)
public:
  CStrAny m_sCmdLine;

  CStartUp()          {}
  virtual ~CStartUp() {}
};

int Main(CStartUp &kStartUp);

#endif