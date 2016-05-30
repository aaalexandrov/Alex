#include "stdafx.h"
#include "Vk.h"
#include "App.h"
#include "Wnd.h"
#include "VGraphics.h"

int Main(App &app)
{
  app.m_name = "VGraphics";
  app.m_ver = 1;

  Window *wndMain = app.NewWindow("Vulkan Graphics");
  if (!wndMain->Init())
    return -1;

  VGraphics vg(true);

  if (!vg.Init())
    return -1;

  while (app.ProcessMessages()) {

  }

  vg.Done();

  return 0;
}

