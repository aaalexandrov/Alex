#include "platform_win32.h"
#include "window_win32.h"

namespace platform {

PlatformWin32::PlatformWin32()
{
  _hInstance = GetModuleHandle(nullptr);
  _windowClass = WindowWin32::RegisterWindowClass(_hInstance);
}

void PlatformWin32::Update()
{
  Platform::Update();
  RunMessageLoop();
}

Window *PlatformWin32::CreateWindowInternal()
{
  return new WindowWin32();
}

void PlatformWin32::RunMessageLoop()
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

std::string PlatformWin32::ToUtf8(std::wstring const &wstr)
{
  std::string str;
  str.resize(wstr.size() * 4);
  int newLen = WideCharToMultiByte(CP_UTF8, 0, 
    wstr.data(), static_cast<int>(wstr.size()), 
    const_cast<char*>(str.data()), static_cast<int>(str.size()), 
    nullptr, nullptr);
  str.resize(newLen);
  return str;
}

std::wstring PlatformWin32::ToWChar(std::string const &str)
{
  std::wstring wstr;
  wstr.resize(str.size());
  int len = MultiByteToWideChar(CP_UTF8, 0, 
    str.data(), static_cast<int>(str.size()), 
    const_cast<wchar_t*>(wstr.data()), static_cast<int>(wstr.size()));
  wstr.resize(len);
  return wstr;
}

}