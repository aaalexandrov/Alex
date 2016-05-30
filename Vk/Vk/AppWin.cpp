#include "stdafx.h"
#include "AppWin.h"
#include "WndWin.h"

#include <iostream>

AppWin::AppWin(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  m_hInst = hInstance;
  m_hPrevInst = hPrevInstance;
  m_cmdShow = nCmdShow;
  ParseArgs(lpCmdLine);

  std::cerr.rdbuf(&m_charDebugOutput);
}

AppWin::~AppWin()
{
  std::cerr.rdbuf();
}

Window *AppWin::NewWindow(std::string const &wndName)
{
  Window *win = new WndWin(wndName, ++m_windowID);
  m_windows.push_back(win);
  return win;
}

bool AppWin::ProcessMessages()
{
  MSG msg;

  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
  {
    if (msg.message == WM_QUIT)
      return false;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return true;
}


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
  AppWin app(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
  int ret = Main(app);
  return ret;
}






