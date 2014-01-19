#include "stdafx.h"
#include "WinInput.h"

#ifdef WINDOWS

// CWinInput ------------------------------------------------------------------

CRTTIRegisterer<CWinInput> g_RegWinInput;

CWinInput::CWinInput()
{
  InitTables();
}

UINT CWinInput::GetModifiersPressed()
{
  UINT uiModifiers = 0;
  if (m_bKeyPressed[KC_LSHIFT] || m_bKeyPressed[KC_RSHIFT])
    uiModifiers |= IM_SHIFT;
  if (m_bKeyPressed[KC_LCTRL] || m_bKeyPressed[KC_RCTRL])
    uiModifiers |= IM_CONTROL;
  if (m_bKeyPressed[KC_LALT] || m_bKeyPressed[KC_RALT])
    uiModifiers |= IM_ALT;

  return uiModifiers;
}

bool CWinInput::IsKeyPressed(int iKey)
{
  ASSERT(iKey < ARRSIZE(m_bKeyPressed));
  return m_bKeyPressed[iKey];
}

bool CWinInput::GetKeyboardFocus()
{
  return GetHWnd() == GetFocus();
}

void CWinInput::SetKeyboardFocus(bool bFocus)
{
  SetFocus(bFocus ? GetHWnd() : 0);
}

CVector<2, int> CWinInput::GetMousePos()
{
  POINT pt;
  GetCursorPos(&pt);
  return CVector<2, int>::Get(pt.x, pt.y);
}

void CWinInput::SetMousePos(CVector<2, int> vPos)
{
  SetCursorPos(vPos.x(), vPos.y());
}

bool CWinInput::GetMouseCapture()
{
  return GetHWnd() == GetCapture();
}

void CWinInput::SetMouseCapture(bool bCapture)
{
  if (bCapture)
    SetCapture(GetHWnd());
  else
    ReleaseCapture();
}

void CWinInput::SetMouseClip(const CRect<int> *pClipRect)
{
  RECT rc;
  if  (pClipRect) {
    m_rcClip = *pClipRect;
    rc.left = m_rcClip.m_vMin.x();
    rc.top = m_rcClip.m_vMin.y();
    rc.right = m_rcClip.m_vMax.x() + 1;
    rc.bottom = m_rcClip.m_vMax.y() + 1;
    ClipCursor(&rc);
  } else {
    m_rcClip.SetEmpty();
    ClipCursor(0);
  }
}

CWinInput::CEvent *CWinInput::MakeMouseEvent(EEventType eEvent, int iKey, WPARAM wParam, LPARAM lParam)
{
  UINT uiModifiers = 0;
  CEvent *pEvent;
  if (wParam & MK_CONTROL)
    uiModifiers |= IM_CONTROL;
  if (wParam & MK_SHIFT)
    uiModifiers |= IM_SHIFT;
  if (m_bKeyPressed[KC_LALT] || m_bKeyPressed[KC_RALT])
    uiModifiers |= IM_ALT;
  pEvent = new CEvent(eEvent, iKey, uiModifiers, m_kTimer.GetCurrent());
  pEvent->m_vPos = CVector<2, int>::Get(LOWORD(lParam), HIWORD(lParam));
  return pEvent;
}

#ifndef MAPVK_VSC_TO_VK_EX // Fuck MinGW?
#define MAPVK_VSC_TO_VK_EX (3)
#endif

bool CWinInput::InputWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  CEvent *pEvent;
  int iKey;
  switch (uMsg) {
    case WM_SYSKEYDOWN:
      if (wParam != VK_MENU && wParam != VK_F10)
        return false;
    case WM_KEYDOWN:
      if (lParam & 0x40000000)
        return false;
      if (wParam == VK_SHIFT) // Map shift to left / right shift using the scancode contained in lParam
        wParam = MapVirtualKey((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
      iKey = VK2Key(wParam, !!(lParam & (1 << 24)));
      m_bKeyPressed[iKey] = false;
      pEvent = new CEvent(ET_KEYDOWN, iKey, GetModifiersPressed(), m_kTimer.GetCurrent());
      break;
    case WM_SYSKEYUP:
      if (wParam != VK_MENU && wParam != VK_F10)
        return false;
    case WM_KEYUP:
      if (wParam == VK_SHIFT) // Map shift to left / right shift using the scancode contained in lParam
        wParam = MapVirtualKey((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX);
      iKey = VK2Key(wParam, !!(lParam & (1 << 24)));
      pEvent = new CEvent(ET_KEYUP, iKey, GetModifiersPressed(), m_kTimer.GetCurrent());
      break;
    case WM_CHAR:
      pEvent = new CEvent(ET_KEYCHAR, (int) wParam, GetModifiersPressed(), m_kTimer.GetCurrent());
      break;
    case WM_MOUSEMOVE:
      pEvent = MakeMouseEvent(ET_MOUSEMOVE, KC_INVALID, wParam, lParam);
      break;
    case WM_LBUTTONDOWN:
      m_bKeyPressed[KC_LMOUSE] = true;
      pEvent = MakeMouseEvent(ET_MOUSEDOWN, KC_LMOUSE, wParam, lParam);
      break;
    case WM_RBUTTONDOWN:
      m_bKeyPressed[KC_RMOUSE] = true;
      pEvent = MakeMouseEvent(ET_MOUSEDOWN, KC_RMOUSE, wParam, lParam);
      break;
    case WM_MBUTTONDOWN:
      m_bKeyPressed[KC_MMOUSE] = true;
      pEvent = MakeMouseEvent(ET_MOUSEDOWN, KC_MMOUSE, wParam, lParam);
      break;
    case WM_LBUTTONUP:
      m_bKeyPressed[KC_LMOUSE] = false;
      pEvent = MakeMouseEvent(ET_MOUSEUP, KC_LMOUSE, wParam, lParam);
      break;
    case WM_RBUTTONUP:
      m_bKeyPressed[KC_RMOUSE] = false;
      pEvent = MakeMouseEvent(ET_MOUSEUP, KC_RMOUSE, wParam, lParam);
      break;
    case WM_MBUTTONUP:
      m_bKeyPressed[KC_MMOUSE] = false;
      pEvent = MakeMouseEvent(ET_MOUSEUP, KC_MMOUSE, wParam, lParam);
      break;
    case WM_LBUTTONDBLCLK:
      pEvent = MakeMouseEvent(ET_MOUSEDOUBLECLICK, KC_LMOUSE, wParam, lParam);
      break;
    case WM_RBUTTONDBLCLK:
      pEvent = MakeMouseEvent(ET_MOUSEDOUBLECLICK, KC_RMOUSE, wParam, lParam);
      break;
    case WM_MBUTTONDBLCLK:
      pEvent = MakeMouseEvent(ET_MOUSEDOUBLECLICK, KC_MMOUSE, wParam, lParam);
      break;
    case WM_MOUSEWHEEL:
      pEvent = MakeMouseEvent(ET_MOUSEWHEEL, HIWORD(wParam), wParam, lParam);
      break;
    default:
      return false;
  }

  BroadcastEvent(pEvent);
  return true;
}

void CWinInput::InitTables()
{
  struct {
    int iKey;
    int iVK;
    bool bExtended;
  } arrKC2VK[] = {
    { CInput::KC_INVALID,      0,             false, },
    { ' ',        VK_SPACE,      false, },
    { '\n',        VK_RETURN,     false, },
    { CInput::KC_ESCAPE,       VK_ESCAPE,     false, },
    { CInput::KC_LSHIFT,       VK_LSHIFT,     false, },
    { CInput::KC_LCTRL,        VK_CONTROL,   false, },
    { CInput::KC_LALT,         VK_MENU,      false, },
    { CInput::KC_LWIN,         VK_LWIN,       true, },
    { CInput::KC_CAPSLOCK,     VK_CAPITAL,    false, },
    { '\t',          VK_TAB,        false, },
    { CInput::KC_RSHIFT,       VK_RSHIFT,     false, },
    { CInput::KC_RCTRL,        VK_CONTROL,   true, },
    { CInput::KC_RALT,         VK_MENU,      true, },
    { CInput::KC_RWIN,         VK_RWIN,       true, },
    { '\b',    VK_BACK,       false, },
    { CInput::KC_INS,          VK_INSERT,     true, },
    { CInput::KC_DEL,          VK_DELETE,     true, },
    { CInput::KC_HOME,         VK_HOME,       true, },
    { CInput::KC_END,          VK_END,        true, },
    { CInput::KC_PAGEUP,       VK_PRIOR,      true, },
    { CInput::KC_PAGEDOWN,     VK_NEXT,       true, },
    { CInput::KC_SCROLLLOCK,   VK_SCROLL,     false, },
    { CInput::KC_NUMLOCK,      VK_NUMLOCK,    true, },
    { CInput::KC_PRINTSCREEN,  VK_PRINT,      false, },
    { CInput::KC_PAUSE,        VK_PAUSE,      false, },
    { CInput::KC_UP,           VK_UP,         true, },
    { CInput::KC_DOWN,         VK_DOWN,       true, },
    { CInput::KC_LEFT,         VK_LEFT,       true, },
    { CInput::KC_RIGHT,        VK_RIGHT,      true, },
    { CInput::KC_NUMPAD | '\n',     VK_RETURN,     true, },
    { CInput::KC_NUMPAD | '.',       VK_DELETE,    false, },
    { CInput::KC_NUMPAD | '0',         VK_INSERT,    false, },
    { CInput::KC_NUMPAD | '1',         VK_END,    false, },
    { CInput::KC_NUMPAD | '2',         VK_DOWN,    false, },
    { CInput::KC_NUMPAD | '3',         VK_NEXT,    false, },
    { CInput::KC_NUMPAD | '4',         VK_LEFT,    false, },
    { CInput::KC_NUMPAD | '5',         VK_CLEAR,    false, },
    { CInput::KC_NUMPAD | '6',         VK_RIGHT,    false, },
    { CInput::KC_NUMPAD | '7',         VK_HOME,    false, },
    { CInput::KC_NUMPAD | '8',         VK_UP,    false, },
    { CInput::KC_NUMPAD | '9',         VK_PRIOR,    false, },
    { CInput::KC_NUMPAD | '+',       VK_ADD,        false, },
    { CInput::KC_NUMPAD | '-',       VK_SUBTRACT,   false, },
    { CInput::KC_NUMPAD | '*',       VK_MULTIPLY,   false, },
    { CInput::KC_NUMPAD | '/',       VK_DIVIDE,     true, },
    { CInput::KC_NUMPAD | '.',       VK_DECIMAL,    false, },
    { CInput::KC_NUMPAD | '0',         VK_NUMPAD0,    false, },
    { CInput::KC_NUMPAD | '1',         VK_NUMPAD1,    false, },
    { CInput::KC_NUMPAD | '2',         VK_NUMPAD2,    false, },
    { CInput::KC_NUMPAD | '3',         VK_NUMPAD3,    false, },
    { CInput::KC_NUMPAD | '4',         VK_NUMPAD4,    false, },
    { CInput::KC_NUMPAD | '5',         VK_NUMPAD5,    false, },
    { CInput::KC_NUMPAD | '6',         VK_NUMPAD6,    false, },
    { CInput::KC_NUMPAD | '7',         VK_NUMPAD7,    false, },
    { CInput::KC_NUMPAD | '8',         VK_NUMPAD8,    false, },
    { CInput::KC_NUMPAD | '9',         VK_NUMPAD9,    false, },
    { CInput::KC_NUMPAD | '+',       VK_ADD,        false, },
    { CInput::KC_NUMPAD | '-',       VK_SUBTRACT,   false, },
    { CInput::KC_NUMPAD | '*',       VK_MULTIPLY,   false, },
    { '`',        VK_OEM_3,      false, },
    { '\\',    VK_OEM_5,      false, },
    { '-',          VK_OEM_MINUS,   false, },
    { '+',       VK_OEM_PLUS,   false, },
    { '[',     VK_OEM_4,      false, },
    { ']',     VK_OEM_6,      false, },
    { ';',    VK_OEM_1,      false, },
    { '\'',    VK_OEM_7,      false, },
    { '\'',        VK_OEM_5,      false, },
    { ',',        VK_OEM_COMMA,  false, },
    { '.',          VK_OEM_PERIOD, false, },
    { '/',        VK_OEM_2,      false, },
    { CInput::KC_102, VK_OEM_102, false },
    { CInput::KC_F1,           VK_F1,         false, },
    { CInput::KC_F2,           VK_F2,         false, },
    { CInput::KC_F3,           VK_F3,         false, },
    { CInput::KC_F4,           VK_F4,         false, },
    { CInput::KC_F5,           VK_F5,         false, },
    { CInput::KC_F6,           VK_F6,         false, },
    { CInput::KC_F7,           VK_F7,         false, },
    { CInput::KC_F8,           VK_F8,         false, },
    { CInput::KC_F9,           VK_F9,         false, },
    { CInput::KC_F10,          VK_F10,        false, },
    { CInput::KC_F11,          VK_F11,        false, },
    { CInput::KC_F12,          VK_F12,        false, },
    { '1',           '1',            false, },
    { '2',           '2',            false, },
    { '3',           '3',            false, },
    { '4',           '4',            false, },
    { '5',           '5',            false, },
    { '6',           '6',            false, },
    { '7',           '7',            false, },
    { '8',           '8',            false, },
    { '9',           '9',            false, },
    { 'A',           'A',            false, },
    { 'B',           'B',            false, },
    { 'C',           'C',            false, },
    { 'D',           'D',            false, },
    { 'E',           'E',            false, },
    { 'F',           'F',            false, },
    { 'G',           'G',            false, },
    { 'H',           'H',            false, },
    { 'I',           'I',            false, },
    { 'J',           'J',            false, },
    { 'K',           'K',            false, },
    { 'L',           'L',            false, },
    { 'M',           'M',            false, },
    { 'N',           'N',            false, },
    { 'O',           'O',            false, },
    { 'P',           'P',            false, },
    { 'Q',           'Q',            false, },
    { 'R',           'R',            false, },
    { 'S',           'S',            false, },
    { 'T',           'T',            false, },
    { 'U',           'U',            false, },
    { 'V',           'V',            false, },
    { 'W',           'W',            false, },
    { 'X',           'X',            false, },
    { 'Y',           'Y',            false, },
    { 'Z',           'Z',            false, },
    { CInput::KC_LMOUSE,      VK_LBUTTON,     false, },
    { CInput::KC_RMOUSE,      VK_RBUTTON,     false, },
    { CInput::KC_MMOUSE,      VK_MBUTTON,     false, },
    { CInput::KC_MOUSE4,      VK_XBUTTON1,    false, },
    { CInput::KC_MOUSE5,      VK_XBUTTON2,    false, },
  };

  memset(m_iKey2VK, 0, sizeof(m_iKey2VK));
  memset(m_iVK2Key, 0, sizeof(m_iVK2Key));
  memset(m_bKeyPressed, 0, sizeof(m_bKeyPressed));

  for (int i = 0; i < ARRSIZE(arrKC2VK); ++i) {
    int iKey = arrKC2VK[i].iKey;
    int iVK = arrKC2VK[i].iVK;
    if (arrKC2VK[i].bExtended)
      iVK += 256;
    if (!m_iKey2VK[iKey])
      m_iKey2VK[iKey] = iVK;
    m_iVK2Key[iVK] = iKey;
  }
}

int CWinInput::Key2VK(int iKey)
{
  return m_iKey2VK[iKey];
}

int CWinInput::VK2Key(int iVK, bool bExtended)
{
  if (bExtended)
    iVK += 256;
  return m_iVK2Key[iVK];
}

#endif // WINDOWS
