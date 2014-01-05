#include "stdafx.h"
#include "WinOSWindow.h"
#include "Str.h"
#include "WinInput.h"

// CWinOSWindow ---------------------------------------------------------------

CRTTIRegisterer<CWinOSWindow> g_RegWinOSWindow;

int CWinOSWindow::s_iWinClass = 1;
CWinOSWindow::TWindowHash CWinOSWindow::s_kWinOSWindows;

CWinOSWindow::CWinOSWindow(HINSTANCE hInstance, int iShowCmd, char const *chClassName)
{
  m_hInstance = hInstance;
  m_iShowCmd = iShowCmd;
  m_hWnd = 0;
  m_Rect.SetEmpty();
}

CWinOSWindow::~CWinOSWindow()
{
  Done();
}

bool CWinOSWindow::Init(CStrAny sName, CRect<int> const *rc)
{
  if (rc)
    m_Rect = *rc;

	WNDCLASSEX wc;
  CStrAny sClassName(ST_WHOLE, "WinOSWindowClass");
  sClassName += CStrAny(ST_STR, s_iWinClass++);

  wc.cbSize = sizeof(wc);
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = CWinOSWindow::WindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = m_hInstance;
  wc.hIcon = 0;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = 0; //(HBRUSH) (COLOR_BACKGROUND + 1);
  wc.lpszMenuName = 0;
  wc.lpszClassName = sClassName.m_pBuf;
  wc.hIconSm = 0;

  if (!RegisterClassEx(&wc)) 
    return false;

  int x = CW_USEDEFAULT, y = CW_USEDEFAULT, w = CW_USEDEFAULT, h = CW_USEDEFAULT;
  CRect<int> rcWindow;
  rcWindow = m_Rect;
  if (!rcWindow.IsEmpty()) {
    if (rcWindow.m_vMin.x() < 0) {
      rcWindow.m_vMax.x() -= rcWindow.m_vMin.x();
      rcWindow.m_vMin.x() = 0;
    }
    if (rcWindow.m_vMin.y() < 0) {
      rcWindow.m_vMax.y() -= rcWindow.m_vMin.y();
      rcWindow.m_vMin.y() = 0;
    }
    Client2WindowRect(rcWindow, rcWindow);
  }
  if (m_Rect.m_vMin.x() >= 0)
    x = rcWindow.m_vMin.x();
  if (m_Rect.m_vMin.y() >= 0)
    y = rcWindow.m_vMin.y();
  if (m_Rect.GetWidth() > 0)
    w = rcWindow.GetWidth();
  if (m_Rect.GetHeight() > 0)
    h = rcWindow.GetHeight();

  m_hWnd = CreateWindowEx(GetWindowStyleEx(), sClassName.m_pBuf, sName.m_pBuf, GetWindowStyle(), x, y, w, h, 0, 0, m_hInstance, 0);
  if (!m_hWnd) 
    return false;

  s_kWinOSWindows.Add(this);

  ShowWindow(m_hWnd, m_iShowCmd);
  UpdateWindow(m_hWnd);

  UpdateRect();
  return true;
}

void CWinOSWindow::Done()
{
  if (!m_hWnd)
    return;

  DestroyWindow(m_hWnd);
  s_kWinOSWindows.RemoveValue(this);
  m_hWnd = 0;
}

CRect<int> const &CWinOSWindow::GetRect() const
{
  return m_Rect;
}

void CWinOSWindow::SetRect(CRect<int> const &rc)
{
  CRect<int> rcWindow;
  Client2WindowRect(rc, rcWindow);
  MoveWindow(m_hWnd, rcWindow.m_vMin.x(), rcWindow.m_vMin.y(), rcWindow.GetWidth(), rcWindow.GetHeight(), true);
  UpdateRect();
}

void CWinOSWindow::UpdateRect()
{
  RECT rcWin;
  GetClientRect(m_hWnd, &rcWin);
  m_Rect.Set(rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
  ASSERT(!m_Rect.IsEmpty());
}

void CWinOSWindow::OnMove(CRect<int> const &rcNew)
{
}

bool CWinOSWindow::OnDraw(CRect<int> const &rcDirty) 
{ 
  return false; 
}

bool CWinOSWindow::Process()
{
  MSG msg;

  bool bContinue = true;
  while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      bContinue = false;
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return bContinue;
}

LRESULT CALLBACK CWinOSWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  CWinInput *pWinInput = (CWinInput *) CInput::Get();
  if (pWinInput && pWinInput->InputWindowProc(hwnd, uMsg, wParam, lParam))
    return 0;
  switch (uMsg) {
    case WM_CLOSE:
      DestroyWindow(hwnd);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_PAINT: {
      RECT rect;
      if (GetUpdateRect(hwnd, &rect, false)) {
        TWindowHash::TIter it = s_kWinOSWindows.Find(hwnd);
        ASSERT(it);
        CRect<int> rcUpdate(rect.left, rect.top, rect.right, rect.bottom);
        it->OnDraw(rcUpdate);
      }
      ValidateRect(hwnd, 0);
      break;
    } 
    case WM_WINDOWPOSCHANGED: {
      TWindowHash::TIter it = s_kWinOSWindows.Find(hwnd);
      ASSERT(it);
      WINDOWPOS *pWinPos = (WINDOWPOS *) lParam;
      CRect<int> rcNew(pWinPos->x, pWinPos->y, pWinPos->x + pWinPos->cx, pWinPos->y + pWinPos->cy);
      if (rcNew != it->m_Rect)
        it->OnMove(rcNew);
      return 0;
    } 
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  return 0;
}

void CWinOSWindow::Client2WindowRect(CRect<int> const &rcClient, CRect<int> &rcWindow)
{
  RECT rcWin;
  rcWin.left = rcClient.m_vMin.x();
  rcWin.top = rcClient.m_vMin.y();
  rcWin.right = rcClient.m_vMax.x();
  rcWin.bottom = rcClient.m_vMax.y();

  AdjustWindowRectEx(&rcWin, GetWindowStyle(), false, GetWindowStyleEx());

  rcWindow.Set(rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
}
