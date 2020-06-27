#pragma once

#include <string>
#include <functional>
#include "util/geom.h"
#include "resource.h"
#include "input.h"
#include "util/namespace.h"

NAMESPACE_BEGIN(platform)

class Platform;

class Window : public Resource {
public:
  enum class Style {
    Invalid,
    CaptionedResizeable,
    BorderlessFullscreen,
  };

  typedef util::RectI Rect;
  typedef std::function<void(Window*, Rect rect)> RectUpdatedFunc;

  RectUpdatedFunc _rectUpdated;
  Rect _lastNotifyRect;
  Input _input;

  Platform *GetPlatform();
  Input &GetInput() { return _input; }

  virtual void Init() = 0;

  virtual void SetRectUpdatedFunc(RectUpdatedFunc func) { _rectUpdated = func; }
  void NotifyRectUpdated();

  virtual Style GetStyle() = 0;
  virtual void SetStyle(Style style) = 0;

  virtual bool IsShown() = 0;
  virtual void SetShown(bool shown) = 0;

  virtual Rect GetRect() = 0;
  virtual void SetRect(Rect const &rect) = 0;

  virtual std::string GetName() = 0;
  virtual void SetName(std::string const &name) = 0;

  virtual bool ShouldClose() = 0;
  virtual void Update();
};

NAMESPACE_END(platform)