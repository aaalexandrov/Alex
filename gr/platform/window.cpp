#include "window.h"
#include "platform.h"

NAMESPACE_BEGIN(platform)

Platform *Window::GetPlatform()
{
  return static_cast<Platform*>(_holder);
}

void Window::NotifyRectUpdated() 
{ 
  if (_rectUpdated != nullptr) {
    Rect rc = GetRect();
    if (rc != _lastNotifyRect) {
      _lastNotifyRect = rc;
      _rectUpdated(this, rc);
    }
  }
}

void Window::Update()
{
  _input.Update();
}

NAMESPACE_END(platform)