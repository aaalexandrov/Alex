#include "window_win32.h"
#include "util/dbg.h"
#include <array>

#include <iostream>


namespace platform {

WCHAR const * const WindowWin32::WindowClass = L"PlatformWindowWin32";

std::unordered_map<int32_t, Key> WindowWin32::VK2Key({
  { VK_LBUTTON, Key::LButton },
  { VK_RBUTTON, Key::RButton },
  { VK_MBUTTON, Key::MButton },
  { VK_XBUTTON1, Key::XButton1 },
  { VK_XBUTTON2, Key::XButton2 },
});

std::unordered_map<int32_t, Key> WindowWin32::ScanCode2Key({
  { 15, Key::Tab },
  { 28, Key::Enter },
  { 284, Key::Enter | Key::Numpad },
  { 57, Key::Space },
  { 11, Key::Key0 },
  { 2, Key::Key1 },
  { 3, Key::Key2 },
  { 4, Key::Key3 },
  { 5, Key::Key4 },
  { 6, Key::Key5 },
  { 7, Key::Key6 },
  { 8, Key::Key7 },
  { 9, Key::Key8 },
  { 10, Key::Key9 },
  { 82, Key::Key0 | Key::Numpad },
  { 79, Key::Key1 | Key::Numpad },
  { 80, Key::Key2 | Key::Numpad },
  { 81, Key::Key3 | Key::Numpad },
  { 75, Key::Key4 | Key::Numpad },
  { 76, Key::Key5 | Key::Numpad },
  { 77, Key::Key6 | Key::Numpad },
  { 71, Key::Key7 | Key::Numpad },
  { 72, Key::Key8 | Key::Numpad },
  { 73, Key::Key9 | Key::Numpad },
  { 30, Key::A },
  { 48, Key::B },
  { 46, Key::C },
  { 32, Key::D },
  { 18, Key::E },
  { 33, Key::F },
  { 34, Key::G },
  { 35, Key::H },
  { 23, Key::I },
  { 36, Key::J },
  { 37, Key::K },
  { 38, Key::L },
  { 50, Key::M },
  { 49, Key::N },
  { 24, Key::O },
  { 25, Key::P },
  { 16, Key::Q },
  { 19, Key::R },
  { 31, Key::S },
  { 20, Key::T },
  { 22, Key::U },
  { 47, Key::V },
  { 17, Key::W },
  { 45, Key::X },
  { 21, Key::Y },
  { 44, Key::Z },
  { 12, Key::Minus },
  { 74, Key::Minus | Key::Numpad },
  { 13, Key::Plus },
  { 78, Key::Plus | Key::Numpad },
  { 26, Key::LeftBracket },
  { 27, Key::RightBracket },
  { 39, Key::SemiColon },
  { 40, Key::Apostrophe },
  { 43, Key::Backslash },
  { 51, Key::Comma },
  { 52, Key::Dot },
  { 83, Key::Dot | Key::Numpad },
  { 53, Key::Slash },
  { 309, Key::Slash | Key::Numpad },
  { 55, Key::Asterisk | Key::Numpad },
  { 41, Key::Tilde },
  { 14, Key::Backspace },
  { 69, Key::Pause },
  { 58, Key::CapsLock },
  { 1, Key::Escape },
  { 329, Key::PageUp },
  { 337, Key::PageDown },
  { 335, Key::End },
  { 327, Key::Home },
  { 331, Key::Left },
  { 328, Key::Up },
  { 333, Key::Right },
  { 336, Key::Down },
  { 55, Key::PrintScreen },
  { 338, Key::Insert },
  { 339, Key::Delete },
  { 325, Key::NumLock },
  { 70, Key::ScrollLock },
  { 347, Key::LWin },
  { 348, Key::RWin },
  { 42, Key::LShift },
  { 54, Key::RShift },
  { 29, Key::LControl },
  { 285, Key::RControl },
  { 56, Key::LAlt },
  { 312, Key::RAlt },
  { 349, Key::ContextMenu },
  { 59, Key::F1 },
  { 60, Key::F2 },
  { 61, Key::F3 },
  { 62, Key::F4 },
  { 63, Key::F5 },
  { 64, Key::F6 },
  { 65, Key::F7 },
  { 66, Key::F8 },
  { 67, Key::F9 },
  { 68, Key::F10 },
  { 87, Key::F11 },
  { 88, Key::F12 }
});

WindowWin32::~WindowWin32()
{
  Destroy();
}

void WindowWin32::Init()
{
  _hWnd = CreateWindowEx(
    0,
    WindowClass,
    L"Platform Window",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    nullptr,
    nullptr,
    GetPlatformWin32()->_hInstance,
    this);

  ASSERT(_hWnd);

  SetWindowLongPtr(_hWnd, 0, reinterpret_cast<LONG_PTR>(this));
}

void WindowWin32::Destroy()
{
  if (!_hWnd)
    return;
  DestroyWindow(_hWnd);
  _hWnd = nullptr;
}

Window::Style WindowWin32::GetStyle()
{
  uint32_t winStyle = static_cast<uint32_t>(GetWindowLongPtr(_hWnd, GWL_STYLE));
  const uint32_t mask = WS_OVERLAPPEDWINDOW | WS_POPUP;
  switch (winStyle & mask) {
    case WS_OVERLAPPEDWINDOW:
      return Style::CaptionedResizeable;
    case WS_POPUP:
      return Style::BorderlessFullscreen;
    default:
      ASSERT(!"Invalid window style received from window");
      return Style::Invalid;
  }
}

void WindowWin32::SetStyle(Style style)
{
  uint32_t winStyle;
  switch (style) {
    case Style::CaptionedResizeable:
      winStyle = WS_OVERLAPPEDWINDOW;
      break;
    case Style::BorderlessFullscreen:
      winStyle = WS_POPUP;
      break;
    default:
      ASSERT(!"Invalid window style set attempt");
      winStyle = WS_OVERLAPPEDWINDOW;
      break;
  }
  const uint32_t mask = WS_OVERLAPPEDWINDOW | WS_POPUP;
  uint32_t currentStyle = static_cast<uint32_t>(GetWindowLongPtr(_hWnd, GWL_STYLE));
  winStyle = (winStyle & mask) | (currentStyle & ~mask);
  SetWindowLongPtr(_hWnd, GWL_STYLE, winStyle);
  SetShown(IsShown());
}

bool WindowWin32::IsShown()
{
  bool visible = IsWindowVisible(_hWnd);
  return visible;
}

void WindowWin32::SetShown(bool shown)
{
  int cmdShow = SW_HIDE;
  if (shown) {
    cmdShow = GetStyle() == Style::BorderlessFullscreen ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
  }
  bool res = ShowWindow(_hWnd, cmdShow);
}

Window::Rect WindowWin32::GetRect() 
{
  Rect rc;
  RECT winRect;
  if (GetClientRect(_hWnd, &winRect)) {
    POINT upLeft{ winRect.left, winRect.top };
    POINT downRight{ winRect.right, winRect.bottom };
    ClientToScreen(_hWnd, &upLeft);
    ClientToScreen(_hWnd, &downRight);
    rc._min.x = upLeft.x;
    rc._min.y = upLeft.y;
    rc._max.x = downRight.x - 1;
    rc._max.y = downRight.y - 1;
  }
  return rc;
}

void WindowWin32::SetRect(Rect const &rect)
{
  RECT winRect{rect._min.x, rect._min.y, rect._max.x + 1, rect._max.y + 1};
  WINDOWINFO winInfo;
  GetWindowInfo(_hWnd, &winInfo);
  bool res = AdjustWindowRect(&winRect, winInfo.dwStyle, false);
  res = MoveWindow(_hWnd, winRect.left, winRect.top, winRect.right - winRect.left, winRect.bottom - winRect.top, true);
}

std::string WindowWin32::GetName() 
{
  std::wstring buf;
  buf.resize(1024);
  int len = GetWindowText(_hWnd, const_cast<wchar_t*>(buf.data()), static_cast<int>(buf.size()));
  buf.resize(len);
  return GetPlatformWin32()->ToUtf8(buf);
}

void WindowWin32::SetName(std::string const &name)
{
  std::wstring buf = GetPlatformWin32()->ToWChar(name);
  SetWindowText(_hWnd, buf.data());
}

Key WindowWin32::GetKeyFromScanCode(int32_t scanCode)
{
  auto it = ScanCode2Key.find(scanCode);
  if (it == ScanCode2Key.end())
    return Key::Invalid;
  return it->second;
}

Key WindowWin32::GetKeyFromVirtualKey(uint32_t vk)
{
  auto it = VK2Key.find(vk);
  if (it == VK2Key.end())
    return Key::Invalid;
  return it->second;
}

ATOM WindowWin32::RegisterWindowClass(HINSTANCE hInst)
{
  WNDCLASS wndClass = {};
  wndClass.lpfnWndProc = WndProc;
  wndClass.hInstance = hInst;
  wndClass.lpszClassName = WindowClass;
  wndClass.cbWndExtra = sizeof(LONG_PTR);
  return RegisterClass(&wndClass);
}

Key WindowWin32::GetKeyForXButton(WPARAM wParam)
{
  switch (GET_XBUTTON_WPARAM(wParam)) {
    case XBUTTON1:
      return VK2Key[VK_XBUTTON1];
    case XBUTTON2:
      return VK2Key[VK_XBUTTON2];
    default:
      return Key::Invalid;
  }
}

LRESULT WindowWin32::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  WindowWin32 *window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hWnd, 0));
  ASSERT(window == nullptr || window->_hWnd == hWnd);
  switch (uMsg) {
    case WM_CLOSE:
      window->_shouldClose = true;
      break;
    case WM_SIZE:
    case WM_MOVE:
      window->NotifyRectUpdated();
      break;

    case WM_CHAR: {
      int32_t scanCode = (lParam >> 16) & 0x1ff;
      std::string utf8Char = window->GetPlatformWin32()->ToUtf8(std::wstring(1, static_cast<wchar_t>(wParam)));
      window->_input.KeyInput(utf8Char, GetKeyFromScanCode(scanCode));
      break;
    }

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN: {
      bool prevState = (lParam >> 30) & 1;
      if (!prevState) {
        int32_t scanCode = (lParam >> 16) & 0x1ff;
        window->_input.KeyEvent(InputEvent::KeyPress, GetKeyFromScanCode(scanCode));
      }
      break;
    }
    case WM_SYSKEYUP:
    case WM_KEYUP: {
      bool prevState = (lParam >> 30) & 1;
      if (prevState) {
        int32_t scanCode = (lParam >> 16) & 0x1ff;
        window->_input.KeyEvent(InputEvent::KeyRelease, GetKeyFromScanCode(scanCode));
      }
      break;
    }

    case WM_LBUTTONDOWN:
      window->_input.KeyEvent(InputEvent::KeyPress, GetKeyFromVirtualKey(VK_LBUTTON));
      break;
    case WM_RBUTTONDOWN:
      window->_input.KeyEvent(InputEvent::KeyPress, GetKeyFromVirtualKey(VK_RBUTTON));
      break;
    case WM_MBUTTONDOWN:
      window->_input.KeyEvent(InputEvent::KeyPress, GetKeyFromVirtualKey(VK_MBUTTON));
      break;
    case WM_XBUTTONDOWN: 
      window->_input.KeyEvent(InputEvent::KeyPress, GetKeyForXButton(wParam));
      break;
    case WM_LBUTTONUP:
      window->_input.KeyEvent(InputEvent::KeyRelease, GetKeyFromVirtualKey(VK_LBUTTON));
      break;
    case WM_RBUTTONUP:
      window->_input.KeyEvent(InputEvent::KeyRelease, GetKeyFromVirtualKey(VK_RBUTTON));
      break;
    case WM_MBUTTONUP:
      window->_input.KeyEvent(InputEvent::KeyRelease, GetKeyFromVirtualKey(VK_MBUTTON));
      break;
    case WM_XBUTTONUP:
      window->_input.KeyEvent(InputEvent::KeyRelease, GetKeyForXButton(wParam));
      break;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
      TextOut(hdc, 0, 0, L"Hello, Windows!", 15);
      EndPaint(hWnd, &ps);
      return 0L;
    }
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

}