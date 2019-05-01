#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef CreateWindow
#undef max
#undef min

#include "../presentation_surface.h"

namespace gr {

struct PresentationSurfaceCreateDataWin32 : public PresentationSurfaceCreateData {
  HINSTANCE _hInstance;
  HWND _hWnd;
};

}