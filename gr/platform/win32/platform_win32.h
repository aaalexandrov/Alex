#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef CreateWindow
#undef LoadImage

#include "../platform.h"

NAMESPACE_BEGIN(platform)

class PlatformWin32 : public Platform {
public:

  PlatformWin32();

  void Update() override;

  Window *CreateWindowInternal() override;

  std::string CurrentDirectory() override;

  void RunMessageLoop();

  std::string ToUtf8(std::wstring const &wstr);
  std::wstring ToWChar(std::string const &str);

  HINSTANCE _hInstance = nullptr;
  ATOM _windowClass = INVALID_ATOM;
};

NAMESPACE_END(platform)