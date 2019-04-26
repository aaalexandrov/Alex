#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef CreateWindow
#undef max
#undef min

#include "../platform.h"

namespace platform {

class PlatformWin32 : public Platform {
public:

  PlatformWin32();

  void Update() override;

  Window *CreateWindowInternal() override;

  void RunMessageLoop();

  std::string ToUtf8(std::wstring const &wstr);
  std::wstring ToWChar(std::string const &str);

  HINSTANCE _hInstance = nullptr;
  ATOM _windowClass = INVALID_ATOM;
};

}