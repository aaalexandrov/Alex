#pragma once

#include "window.h"
#include "resource.h"

NAMESPACE_BEGIN(platform)

class Platform : public ResourceHolder {
public:
  static Platform *Create();

  ~Platform();

  Window* CreateWindow();

  virtual void Update();

  void UpdateWindows();

  virtual Window *CreateWindowInternal() = 0;
};

NAMESPACE_END(platform)