#include <iostream>
#include <chrono>
#include "glm/glm.hpp"
#include "util/rect.h"
#include "util/time.h"
#include "platform/platform.h"
#include "gr/graphics.h"

#if defined(_WIN32)
#include "gr/win32/presentation_surface_create_data_win32.h"
#include "platform/win32/window_win32.h"
#endif

using namespace std;
using namespace glm;
using namespace platform;
using namespace gr;

int main()
{
  auto platform = std::unique_ptr<Platform>(Platform::Create());
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

#if defined(_WIN32)
  auto windowWin32 = dynamic_cast<WindowWin32*>(window);
  PresentationSurfaceCreateDataWin32 surfaceData;
  surfaceData._hInstance = windowWin32->GetPlatformWin32()->_hInstance;
  surfaceData._hWnd = windowWin32->_hWnd;
#endif

  graphics->Init(surfaceData);
  graphics->GetDefaultPresentationSurface()->Update(ri.GetSize().x, ri.GetSize().y);

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