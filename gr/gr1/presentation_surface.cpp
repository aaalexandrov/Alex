#include "presentation_surface.h"

NAMESPACE_BEGIN(gr1)

void PresentationSurface::SetSurfaceFormat(ColorFormat format)
{
	_surfaceFormat = format;
}

void PresentationSurface::SetPresentMode(PresentMode presentMode)
{
	_presentMode = presentMode;
}

ColorFormat PresentationSurface::GetFirstAvailableSurfaceFormat(std::vector<ColorFormat> const &desiredFormats)
{
	return GetFirstAvailableOption(desiredFormats, GetSupportedSurfaceFormats());
}

PresentMode PresentationSurface::GetFirstAvailablePresentMode(std::vector<PresentMode> const &desiredModes)
{
	return GetFirstAvailableOption(desiredModes, GetSupportedPresentModes());
}

NAMESPACE_END(gr1)