#include "render_queue.h"

NAMESPACE_BEGIN(gr)

void RenderQueue::AddResourceUpdate(std::shared_ptr<ResourceUpdate>& resouceUpdate)
{
  _resourceUpdates.push_back(resouceUpdate);
}

void RenderQueue::ProcessResourceUpdates()
{
}


NAMESPACE_END(gr)

