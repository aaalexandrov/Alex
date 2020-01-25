#include <iostream>
#include <chrono>
#include <filesystem>
#include "glm/glm.hpp"
#include "util/rect.h"
#include "util/time.h"
#include "platform/platform.h"
#include "gr1/host.h"
#include "gr1/device.h"

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
#include "gr1/win32/presentation_surface_create_data_win32.h"
#include "platform/win32/window_win32.h"
#endif

using namespace std;
using namespace glm;
using namespace platform;

int main()
{
  auto platform = std::unique_ptr<Platform>(Platform::Create());

  cout << "Current execution directory: " << platform->CurrentDirectory() << endl;

  Window *window = platform->CreateWindow();

  window->SetName("gr test");

  auto ri = window->GetRect();
  ri.SetSize(ivec2(300, 300));
  window->SetRect(ri);

  //window->SetStyle(Window::Style::Borderless);

  window->SetShown(true);

  ri = window->GetRect();

	gr1::Host host;
	std::shared_ptr<gr1::Device> device = host.CreateDevice(0);


#if defined(_WIN32)
  auto windowWin32 = dynamic_cast<WindowWin32*>(window);
  gr1::PresentationSurfaceCreateDataWin32 surfaceData;
  surfaceData._hInstance = windowWin32->GetPlatformWin32()->_hInstance;
  surfaceData._hWnd = windowWin32->_hWnd;
#endif

	auto surface = device->CreateResource<gr1::PresentationSurface>();
	surface->Init(surfaceData);
	surface->Update(ri.GetSize().x, ri.GetSize().y);

	window->SetRectUpdatedFunc([&](Window *window, Window::Rect rect) {
		LOG("Window rect changed ", util::ToString(rect._min), util::ToString(rect._max));
		surface->Update(rect.GetSize().x, rect.GetSize().y);
	});


  {
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
  }

  return 0;
}