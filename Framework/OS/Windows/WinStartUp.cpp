#include "stdafx.h"
#include "WinStartUp.h"

#ifdef WINDOWS

// CWinStartUp ----------------------------------------------------------------
CRTTIRegisterer<CWinStartUp> g_RegWinStartUp;

CWinStartUp::CWinStartUp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
  m_hInstance = hInstance;
  m_hPrevInstance = hPrevInstance;
  m_iCmdShow = iCmdShow;
  m_sCmdLine = lpCmdLine;
}
 
CWinStartUp::~CWinStartUp()
{
}

#ifdef USE_STARTUP

 int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
 {
   CWinStartUp kStartUp(hInstance, hPrevInstance, lpCmdLine, iCmdShow);
   int iRet = Main(kStartUp);
   return iRet;
 }

#endif
 
#endif // WINDOWS