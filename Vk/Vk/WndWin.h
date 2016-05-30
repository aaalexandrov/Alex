#pragma once
#include "Wnd.h"
#include <string>

class WndWin : public Window
{
public:
  std::wstring m_wndClass;
  int m_windowID;
  HWND m_hWnd = 0;

  WndWin(std::string const &name, int id);
  virtual ~WndWin();

  virtual bool Init();
  virtual void Done();

  virtual uintptr_t GetID() { return reinterpret_cast<uintptr_t>(m_hWnd); }
public:
  bool InitClass();
  void DoneClass();

  bool InitWindow();
  void DoneWindow();

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

