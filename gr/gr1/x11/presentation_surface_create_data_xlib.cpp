#include "presentation_surface_create_data_xlib.h"

NAMESPACE_BEGIN(gr1)

VisualID PresentationSurfaceCreateDataXlib::GetVisualId() const
{
    XWindowAttributes winAttr;
    XGetWindowAttributes(_display, _window, &winAttr);
    return XVisualIDFromVisual(winAttr.visual);
}

NAMESPACE_END(gr1)