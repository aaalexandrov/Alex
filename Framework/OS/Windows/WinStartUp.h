#ifndef __WINSTARTUP_H
#define __WINSTARTUP_H

#ifdef WINDOWS

#include "../StartUp.h"
#include <Windows.h>

class CWinStartUp : public CStartUp {
  DEFRTTI(CWinStartUp, CStartUp, false)
public:
  HINSTANCE m_hInstance, m_hPrevInstance;
  int m_iCmdShow;

  CWinStartUp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow);
  virtual ~CWinStartUp();
};

#endif // WINDOWS

#endif