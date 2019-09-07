#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef CreateWindow
#undef CreateSemaphore
#undef LoadImage

#include "../presentation_surface.h"

NAMESPACE_BEGIN(gr)

struct PresentationSurfaceCreateDataWin32 : public PresentationSurfaceCreateData {
  HINSTANCE _hInstance;
  HWND _hWnd;
};

NAMESPACE_END(gr)