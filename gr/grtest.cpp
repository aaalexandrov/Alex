#include <iostream>
#include <chrono>
#include <filesystem>
#include "glm/glm.hpp"
#include "util/rect.h"
#include "util/time.h"
#include "platform/platform.h"
#include "gr/graphics.h"
#include "gr/shader.h"

#if defined(_MSC_VER) && defined(_DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static struct DbgInit {
  DbgInit() 
  {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  }
} dbgInit;

#endif

#if defined(_WIN32)
#include "gr/win32/presentation_surface_create_data_win32.h"
#include "platform/win32/window_win32.h"
#endif

using namespace std;
using namespace glm;
using namespace platform;
using namespace gr;

namespace util {
void Test();
}

int main()
{
  //util::Test();
  auto platform = std::unique_ptr<Platform>(Platform::Create());

  cout << "Current execution directory: " << platform->CurrentDirectory() << endl;

  Window *window = platform->CreateWindow();

  window->SetName("gr test");

  window->SetRectUpdatedFunc([](Window *window, Window::Rect rect) {
    LOG("Window rect changed ", util::ToString(rect._min), util::ToString(rect._max));
  });

  auto ri = window->GetRect();
  ri.SetSize(ivec2(300, 300));
  window->SetRect(ri);

  //window->SetStyle(Window::Style::Borderless);

  window->SetShown(true);

  ri = window->GetRect();

  auto graphics = std::unique_ptr<Graphics>(Graphics::Create());
  graphics->SetLoadPath("../data");

#if defined(_WIN32)
  auto windowWin32 = dynamic_cast<WindowWin32*>(window);
  PresentationSurfaceCreateDataWin32 surfaceData;
  surfaceData._hInstance = windowWin32->GetPlatformWin32()->_hInstance;
  surfaceData._hWnd = windowWin32->_hWnd;
#endif

  graphics->Init(surfaceData);
  graphics->GetRenderQueue()->GetPresentationSurface()->Update(ri.GetSize().x, ri.GetSize().y);

  auto shader = graphics->LoadShader("simple");

  auto start = std::chrono::system_clock::now();

  while (!window->ShouldClose()) {
    if (window->GetInput().IsJustPressed(Key::Enter)) {
      auto newStyle = window->GetStyle() == Window::Style::CaptionedResizeable 
        ? Window::Style::BorderlessFullscreen 
        : Window::Style::CaptionedResizeable;

      window->SetStyle(newStyle);
    }

    auto text = window->GetInput()._input;
    if (text.size() > 0) {
      LOG("Input ", text);
    }

    platform->Update();
  }

  LOG("Quitting after ", util::ToSeconds(std::chrono::system_clock::now() - start), " seconds");

  return 0;
}