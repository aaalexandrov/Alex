#include "stdafx.h"
#include "WndWin.h"
#include "AppWin.h"

WndWin::WndWin(std::string const &name, int id) : Window(name)
{
  m_wndClass = L"WndWinClass" + std::to_wstring(id);
}

WndWin::~WndWin()
{
}

bool WndWin::Init()
{
  if (!InitClass())
    return false;
  if (!InitWindow())
    return false;

  return true;
}

void WndWin::Done()
{
  DoneWindow();
  DoneClass();
}

bool WndWin::InitClass()
{
  AppWin *app = AppWin::Get();
  WNDCLASSEX wcex;
  
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = app->m_hInst;
  wcex.hIcon = LoadIcon(app->m_hInst, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = m_wndClass.c_str();
  wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

  return !!RegisterClassExW(&wcex);
}

void WndWin::DoneClass()
{
  bool res = !!UnregisterClassW(m_wndClass.c_str(), AppWin::Get()->m_hInst);
  assert(res);
}

bool WndWin::InitWindow()
{
  AppWin *app = AppWin::Get();
  std::wstring wndName = App::StrToWstr(m_name);

  m_hWnd = CreateWindowW(m_wndClass.c_str(), wndName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, app->m_hInst, nullptr);

  if (!m_hWnd)
    return false;

  ShowWindow(m_hWnd, app->m_cmdShow);
  UpdateWindow(m_hWnd);

  return true;
}

void WndWin::DoneWindow()
{
  DestroyWindow(m_hWnd);
}

LRESULT CALLBACK WndWin::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);

      EndPaint(hWnd, &ps);
    } break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

