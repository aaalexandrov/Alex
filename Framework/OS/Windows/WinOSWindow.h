#ifndef __WINOSWINDOW_H
#define __WINOSWINDOW_H

#ifdef WINDOWS

#include "OSWindow.h"
#include <Windows.h>

class CWinOSWindow : public COSWindow {
  DEFRTTI(CWinOSWindow, COSWindow, true)
public:
  static int s_iWinClass;

  HWND m_hWnd;
  CRect<int> m_Rect;

  CWinOSWindow();
  virtual ~CWinOSWindow();

  virtual bool Init(CStartUp const &kStartUp, CCallback *pCallback, CStrAny sName, CRect<int> const *rc);
  virtual void Done();

  virtual CRect<int> const &GetRect() const;
  virtual void SetRect(CRect<int> const &rc);

  virtual bool Process();
  
public:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  DWORD GetWindowStyle() const   { return WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW; }
  DWORD GetWindowStyleEx() const { return WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW; }
  void Client2WindowRect(CRect<int> const &rcClient, CRect<int> &rcWindow);
  void UpdateRect();

  static inline bool   Eq(CWinOSWindow const *pWindow0, CWinOSWindow const *pWindow1) { return pWindow0->m_hWnd == pWindow1->m_hWnd; }
  static inline bool   Eq(HWND hWnd, CWinOSWindow const *pWindow)                     { return hWnd == pWindow->m_hWnd;              }
  static inline size_t Hash(CWinOSWindow const *pWindow)                              { return (size_t) pWindow->m_hWnd;             }
  static inline size_t Hash(HWND hWnd)                                                { return (size_t) hWnd;                        }

  typedef CHash<CWinOSWindow *, HWND, CWinOSWindow, CWinOSWindow> TWindowHash;
  static TWindowHash s_kWinOSWindows;
};

#endif // WINDOWS

#endif