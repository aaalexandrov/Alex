#pragma once

#include "platform_win32.h"
#include "../window.h"
#include "../input.h"
#include <unordered_map>


NAMESPACE_BEGIN(platform)

class WindowWin32 : public Window {
public:
  static WCHAR const * const WindowClass;
  static std::unordered_map<int32_t, Key> ScanCode2Key;
  static std::unordered_map<int32_t, Key> VK2Key;

  HWND _hWnd = nullptr;
  bool _shouldClose = false;

  PlatformWin32 *GetPlatformWin32() { return static_cast<PlatformWin32*>(GetPlatform()); }

  ~WindowWin32();

  void Init() override;

  void Destroy();

  Style GetStyle() override;
  void  SetStyle(Style style) override;

  bool IsShown() override;
  void SetShown(bool shown) override;

  Rect GetRect() override;
  void SetRect(Rect const &rect) override;

  std::string GetName() override;
  void SetName(std::string const &name) override;

  bool ShouldClose() override { return _shouldClose; }

  static Key GetKeyFromScanCode(int32_t scanCode);
  static Key GetKeyFromVirtualKey(uint32_t vk);

  static ATOM RegisterWindowClass(HINSTANCE hInst);

  static Key GetKeyForXButton(WPARAM wParam);

  static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

NAMESPACE_END(platform)