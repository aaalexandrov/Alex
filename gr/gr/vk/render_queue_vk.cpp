#include "render_queue_vk.h"
#include "device_vk.h"
#include "presentation_surface_vk.h"

NAMESPACE_BEGIN(gr)

void RenderQueueVk::Render()
{
}

PresentationSurfaceVk *RenderQueueVk::GetPresentationSurfaceVk() 
{ 
  return static_cast<PresentationSurfaceVk*>(_presentationSurface.get()); 
}

NAMESPACE_END(gr)

