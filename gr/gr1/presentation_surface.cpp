#include "presentation_surface.h"

NAMESPACE_BEGIN(gr1)

void PresentationSurface::Init(PresentationSurfaceCreateData &createData, PresentMode presentMode)
{
	_presentMode = presentMode;
}

NAMESPACE_END(gr1)