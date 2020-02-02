#include <iostream>
#include <chrono>
#include <filesystem>
#include "glm/glm.hpp"
#include "util/rect.h"
#include "util/time.h"
#include "util/file.h"
#include "platform/platform.h"
#include "gr1/host.h"
#include "gr1/device.h"
#include "gr1/execution_queue.h"
#include "gr1/image.h"
#include "gr1/shader.h"
#include "gr1/render_pass.h"
#include <string>

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
using namespace gr1;

shared_ptr<Shader> LoadShader(Device *device, std::string name)
{
	vector<uint8_t> contents = util::ReadFile(string("../data/") + name);
	string ext = name.substr(name.find_last_of('.'));
	ShaderKind kind = ext == ".vert" ? ShaderKind::Vertex : ShaderKind::Fragment;
	shared_ptr<Shader> shader = device->CreateResource<Shader>();
	shader->Init(name, kind, contents);
	return shader;
}

int main()
{
  auto platform = unique_ptr<Platform>(Platform::Create());

  cout << "Current execution directory: " << platform->CurrentDirectory() << endl;

  Window *window = platform->CreateWindow();

  window->SetName("gr test");

  auto ri = window->GetRect();
  ri.SetSize(ivec2(300, 300));
  window->SetRect(ri);

  //window->SetStyle(Window::Style::Borderless);

  window->SetShown(true);

  ri = window->GetRect();

	Host host;
	shared_ptr<Device> device = host.CreateDevice(0);


#if defined(_WIN32)
  auto windowWin32 = dynamic_cast<WindowWin32*>(window);
  PresentationSurfaceCreateDataWin32 surfaceData;
  surfaceData._hInstance = windowWin32->GetPlatformWin32()->_hInstance;
  surfaceData._hWnd = windowWin32->_hWnd;
#endif

	auto vertShader = LoadShader(device.get(), "simple.vert");
	auto fragShader = LoadShader(device.get(), "simple.frag");

	auto surface = device->CreateResource<PresentationSurface>();
	surface->Init(surfaceData, PresentMode::Immediate);
	surface->Update(ri.GetSize().x, ri.GetSize().y);

	auto depthBuffer = device->CreateResource<Image>();
	depthBuffer->Init(Image::Usage::DepthBuffer, ColorFormat::D24S8, uvec4(ri.GetSize().x, ri.GetSize().y, 0, 0), 1);

	window->SetRectUpdatedFunc([&](Window *window, Window::Rect rect) {
		LOG("Window rect changed ", util::ToString(rect._min), util::ToString(rect._max));
	});

  {
    auto start = chrono::system_clock::now();
		uint32_t frameNumber = 0;

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

			if (surface->GetSize() != glm::uvec2(depthBuffer->GetSize())) {
				auto size = surface->GetSize();
				surface->Update(size.x, size.y);
				depthBuffer = device->CreateResource<Image>();
				depthBuffer->Init(Image::Usage::DepthBuffer, ColorFormat::D24S8, uvec4(size.x, size.y, 0, 0), 1);
			}

			auto backBuffer = surface->AcquireNextImage();
			auto renderPass = device->CreateResource<RenderPass>();
			renderPass->AddAttachment(ContentTreatment::Clear, backBuffer, ContentTreatment::Keep, vec4(0, 0, 1, 1));
			renderPass->AddAttachment(ContentTreatment::Clear, depthBuffer, ContentTreatment::Keep, vec4(1, 0, 0, 0));

			auto presentPass = device->CreateResource<PresentPass>();
			presentPass->Init(surface, backBuffer);

			device->GetExecutionQueue().EnqueuePass(renderPass);
			device->GetExecutionQueue().EnqueuePass(presentPass);
			device->GetExecutionQueue().ExecutePasses();

      platform->Update();
			++frameNumber;
    }

		float seconds = util::ToSeconds(chrono::system_clock::now() - start);
		cout << "Quitting after " << seconds << " seconds and " << frameNumber << " frames for " << frameNumber / seconds << " FPS" << endl;
  }

  return 0;
}