#ifndef __WININPUT_H
#define __WININPUT_H

#include "Input.h"
#include <Windows.h>


class CWinInput: public CInput {
  DEFRTTI(CWinInput, CInput, true)
public:
  int m_iKey2VK[256];
  int m_iVK2Key[512];
  bool m_bKeyPressed[256];

  CWinInput();

  virtual UINT GetModifiersPressed();
  virtual bool IsKeyPressed(int iKey);

  virtual bool GetKeyboardFocus();
  virtual void SetKeyboardFocus(bool bFocus);

  virtual CVector<2, int> GetMousePos();
  virtual void SetMousePos(CVector<2, int> vPos);

  virtual bool GetMouseCapture();
  virtual void SetMouseCapture(bool bCapture);

  virtual void SetMouseClip(const CRect<int> *pClipRect);

public:
  bool InputWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  CEvent *MakeMouseEvent(EEventType eEvent, int iKey, WPARAM wParam, LPARAM lParam);

  void InitTables();
  
  int Key2VK(int iKey);
  int VK2Key(int iVK, bool bExtended);

  HWND GetHWnd() { return (HWND) m_pWindow; }
};


#endif