#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "App.h"
#include "OutputDebugBuf.h"

class AppWin: public App
{
public:
  HINSTANCE m_hInst, m_hPrevInst;
  int m_cmdShow;
  int m_windowID = 0;
  OutputDebugStringBuf<char> m_charDebugOutput;

  AppWin(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
  virtual ~AppWin();

  virtual Window *NewWindow(std::string const &wndName);
  virtual bool AppWin::ProcessMessages();
  virtual uintptr_t GetID() { return reinterpret_cast<uintptr_t>(m_hInst); }

  static AppWin *Get() { return static_cast<AppWin*>(App::Get()); }
};


