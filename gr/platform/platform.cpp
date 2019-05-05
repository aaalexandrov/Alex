#include <memory>
#include "platform.h"

#ifdef WIN32
#include "win32/platform_win32.h"
#endif

NAMESPACE_BEGIN(platform)

Platform *Platform::Create()
{
#ifdef WIN32
  return new PlatformWin32();
#elif
  #error Unsupported platform!
#endif
}

Platform::~Platform()
{
  DestroyResources();
}

void Platform::Update()
{
  UpdateWindows();
}

void Platform::UpdateWindows()
{
  for (auto res : _resources) {
    auto window = dynamic_cast<Window*>(res);
    if (!window)
      continue;
    window->Update();
  }
}

Window *Platform::CreateWindow()
{
  auto window = CreateWindowInternal();
  RegisterResource(window);
  window->Init();
  return window;
}

NAMESPACE_END(platform)