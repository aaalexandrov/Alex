#include "stdafx.h"
#include "Input.h"
#include "Debug.h"

// CInput::CEvent -------------------------------------------------------------

CInput::CEvent::CEvent(EEventType eEvent, int iKey, UINT uiModifiers, CTime kTime)
: m_Time(kTime)
{
  m_eEvent = eEvent;
  m_iKey = iKey;
  m_uiModifiers = uiModifiers;
}


// CInput ---------------------------------------------------------------------

CInput *CInput::s_pInput = 0;

CInput::CInput(HWND hWnd)
{
  ASSERT(!s_pInput);
  s_pInput = this;
  m_hWnd = hWnd;
  m_rcClip.SetEmpty();
}

CInput::~CInput()
{
  ASSERT(s_pInput == this);
  s_pInput = 0;
}

UINT CInput::GetModifiersPressed()
{
  UINT uiModifiers = 0;
  if (IsKeyPressed(VK_SHIFT))
    uiModifiers |= IM_SHIFT;
  if (IsKeyPressed(VK_CONTROL))
    uiModifiers |= IM_CONTROL;
  if (IsKeyPressed(VK_MENU))
    uiModifiers |= IM_ALT;

  return uiModifiers;
}

bool CInput::IsKeyPressed(int iKey)
{
  return !!(0x8000 & GetKeyState(iKey));
}

bool CInput::GetKeyboardFocus()
{
  return m_hWnd == GetFocus();
}

void CInput::SetKeyboardFocus(bool bFocus)
{
  SetFocus(bFocus ? m_hWnd : 0);
}

CVector<2, int> CInput::GetMousePos()
{
  POINT pt;
  GetCursorPos(&pt);
  return CVector<2, int>::Get(pt.x, pt.y);
}

void CInput::SetMousePos(CVector<2, int> vPos)
{
  SetCursorPos(vPos.x(), vPos.y());
}

bool CInput::GetMouseCapture()
{
  return m_hWnd == GetCapture();
}

void CInput::SetMouseCapture(bool bCapture)
{
  if (bCapture)
    SetCapture(m_hWnd);
  else
    ReleaseCapture();
}

const CRect<int> *CInput::GetMouseClip()
{
  return &m_rcClip;
}

void CInput::SetMouseClip(const CRect<int> *pClipRect)
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

void CInput::SetEventListener(CEventListener *pListener, UINT uiPriority)
{
  RemoveEventListener(pListener);
  TListenerList::TNode *pNode;
  for (pNode = m_lstListeners.m_pHead; pNode; pNode = pNode->pNext) 
    if (pNode->Data.uiPriority < uiPriority)
      m_lstListeners.PushBefore(pNode, TListenerInfo(pListener, uiPriority));
  if (!pNode)
    m_lstListeners.PushTail(TListenerInfo(pListener, uiPriority));
}

void CInput::RemoveEventListener(CEventListener *pListener)
{
  m_lstListeners.Remove(m_lstListeners.Find(pListener));
}

void CInput::BroadcastEvent(CEvent *pEvent)
{
  TListenerList::TNode *pNode;
  for (pNode = m_lstListeners.m_pHead; pNode; pNode = pNode->pNext) {
    bool bContinue = pNode->Data.pListener->OnInputEvent(pEvent);
    if (!bContinue)
      break;
  }
  delete pEvent;
}

CInput::CEvent *CInput::MakeMouseEvent(EEventType eEvent, int iKey, WPARAM wParam, LPARAM lParam)
{
  UINT uiModifiers = 0;
  CEvent *pEvent;
  if (wParam & MK_CONTROL)
    uiModifiers |= IM_CONTROL;
  if (wParam & MK_SHIFT)
    uiModifiers |= IM_SHIFT;
  if (IsKeyPressed(VK_MENU))
    uiModifiers |= IM_ALT;
  pEvent = new CEvent(eEvent, iKey, uiModifiers, CTime::GetCurrentReal());
  pEvent->m_vPos = CVector<2, int>::Get(LOWORD(lParam), HIWORD(lParam));
  return pEvent;
}

bool CInput::InputWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  CEvent *pEvent;
  switch (uMsg) {
    case WM_KEYDOWN:
      if (lParam & 0x40000000)
        return false;
      pEvent = new CEvent(ET_KEYDOWN, (int) wParam, GetModifiersPressed(), CTime::GetCurrentReal());
      break;
    case WM_KEYUP:
      pEvent = new CEvent(ET_KEYUP, (int) wParam, GetModifiersPressed(), CTime::GetCurrentReal());
      break;
    case WM_CHAR:
      pEvent = new CEvent(ET_KEYCHAR, (int) wParam, GetModifiersPressed(), CTime::GetCurrentReal());
      break;
    case WM_MOUSEMOVE:
      pEvent = MakeMouseEvent(ET_MOUSEMOVE, 0, wParam, lParam);
      break;
    case WM_LBUTTONDOWN:
      pEvent = MakeMouseEvent(ET_MOUSEDOWN, VK_LBUTTON, wParam, lParam);
      break;
    case WM_RBUTTONDOWN:
      pEvent = MakeMouseEvent(ET_MOUSEDOWN, VK_RBUTTON, wParam, lParam);
      break;
    case WM_MBUTTONDOWN:
      pEvent = MakeMouseEvent(ET_MOUSEDOWN, VK_MBUTTON, wParam, lParam);
      break;
    case WM_LBUTTONUP:
      pEvent = MakeMouseEvent(ET_MOUSEUP, VK_LBUTTON, wParam, lParam);
      break;
    case WM_RBUTTONUP:
      pEvent = MakeMouseEvent(ET_MOUSEUP, VK_RBUTTON, wParam, lParam);
      break;
    case WM_MBUTTONUP:
      pEvent = MakeMouseEvent(ET_MOUSEUP, VK_MBUTTON, wParam, lParam);
      break;
    case WM_LBUTTONDBLCLK:
      pEvent = MakeMouseEvent(ET_MOUSEDOUBLECLICK, VK_LBUTTON, wParam, lParam);
      break;
    case WM_RBUTTONDBLCLK:
      pEvent = MakeMouseEvent(ET_MOUSEDOUBLECLICK, VK_RBUTTON, wParam, lParam);
      break;
    case WM_MBUTTONDBLCLK:
      pEvent = MakeMouseEvent(ET_MOUSEDOUBLECLICK, VK_MBUTTON, wParam, lParam);
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
