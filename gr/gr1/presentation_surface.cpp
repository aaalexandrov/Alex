#include "presentation_surface.h"

NAMESPACE_BEGIN(gr1)

void PresentationSurface::SetPresentMode(PresentMode presentMode)
{
	_presentMode = presentMode;
}

PresentMode PresentationSurface::GetFirstAvailablePresentMode(std::vector<PresentMode> const &desiredModes)
{
	auto availableModes = GetSupportedPresentModes();
	for (PresentMode desired : desiredModes) {
		if (std::find(availableModes.begin(), availableModes.end(), desired) != availableModes.end()) {
			return desired;
		}
	}
	ASSERT(!"Requested present modes not found, defaulting to first available!");
	return availableModes.front();
}

NAMESPACE_END(gr1)