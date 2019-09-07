#include "operation_queue_vk.h"
#include "device_vk.h"
#include "presentation_surface_vk.h"

NAMESPACE_BEGIN(gr)

GraphicsVk *OperationQueueVk::GetGraphics() const  
{ 
  return _device->GetGraphics(); 
}

PresentationSurfaceVk *OperationQueueVk::GetPresentationSurfaceVk()
{ 
  return static_cast<PresentationSurfaceVk*>(_presentationSurface.get()); 
}

void OperationQueueVk::PreProcessOperations()
{
}

void OperationQueueVk::ExecuteOperation(QueueOperation *operation, std::vector<QueueOperation*> const &dependsOn)
{

}

void OperationQueueVk::PostProcessOperations()
{
}

NAMESPACE_END(gr)

