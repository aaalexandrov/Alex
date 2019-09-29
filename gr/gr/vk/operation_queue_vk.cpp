#include "operation_queue_vk.h"
#include "queue_operation_vk.h"
#include "device_vk.h"
#include "presentation_surface_vk.h"

NAMESPACE_BEGIN(gr)

class QueueWait : public QueueOperationVk {
public:
  QueueWait(QueueVk &queue) : _queue(&queue) {}

  void Prepare(OperationQueue *operationQueue) override
  {
    _cmds = _queue->AllocateCmdBuffer();
    _cmds->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    
    _cmds->pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), nullptr, nullptr, nullptr);

    _cmds->end();
  }

  void Execute(OperationQueue *operationQueue) override
  {
    auto *opQueue = static_cast<OperationQueueVk*>(operationQueue);
    std::array<vk::SubmitInfo, 1> submitInfo;
    submitInfo[0]
      .setCommandBufferCount(1)
      .setPCommandBuffers(&_cmds.get());
    _queue->_queue.submit(submitInfo, opQueue->QueueFence(_queue->_role));
  }

  QueueVk *_queue;
  vk::UniqueCommandBuffer _cmds;
};

OperationQueueVk::OperationQueueVk(DeviceVk &device) 
  : _device { &device } 
{
  for (int i = 0; i < _queueFences.size(); ++i) {
    _queueFences[i] = _device->CreateFence();
  }
}

GraphicsVk *OperationQueueVk::GetGraphics() const  
{ 
  return _device->GetGraphics(); 
}

PresentationSurfaceVk *OperationQueueVk::GetPresentationSurfaceVk()
{ 
  return static_cast<PresentationSurfaceVk*>(_presentationSurface.get()); 
}

void OperationQueueVk::ClearOperations()
{
  OperationQueue::ClearOperations();
}

void OperationQueueVk::PreProcessOperations()
{
  for (int i = 0; i < _device->_queues.size(); ++i) {
    auto wait = std::make_unique<QueueWait>(_device->_queues[i]);
    AddOperation(std::move(wait));
  }
}

void OperationQueueVk::PostProcessOperations()
{
}

void OperationQueueVk::WaitOperationsEnd()
{
  std::array<vk::Fence, static_cast<int>(QueueRole::Count)> fences;
  for (int i = 0; i < fences.size(); ++i)
    fences[i] = _queueFences[i].get();
  vk::Result res = _device->_device->waitForFences(fences, true, std::numeric_limits<uint64_t>::max());
  ASSERT(res == vk::Result::eSuccess);
  _device->_device->resetFences(fences);
}


NAMESPACE_END(gr)

