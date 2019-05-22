#include "graphics_resource.h"

NAMESPACE_BEGIN(gr)

ResourceUpdate::ResourceUpdate(GraphicsResource &updatedResource)
  : _resource(updatedResource.shared_from_this())
{
}

NAMESPACE_END(gr)