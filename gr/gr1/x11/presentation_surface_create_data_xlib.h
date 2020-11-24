#pragma once

#include "X11/Xlib.h"
#undef None
#undef Always

#include "../presentation_surface.h"

NAMESPACE_BEGIN(gr1)

struct PresentationSurfaceCreateDataXlib : public PresentationSurfaceCreateData {
	RTTR_ENABLE(PresentationSurfaceCreateData)
public:
  PresentationSurfaceCreateDataXlib(Display *display, Window window) : _display(display), _window(window) {}
  VisualID GetVisualId() const;

  Display *_display;
  Window _window;
};

NAMESPACE_END(gr1)