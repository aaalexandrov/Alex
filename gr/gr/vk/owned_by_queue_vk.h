#pragma once

#include "queue_vk.h"
#include "device_vk.h"

NAMESPACE_BEGIN(gr)

class OperationQueue;

template <typename Resource>
struct OwnedByQueueVk {
  struct QueueTransition {
    QueueTransition(Resource *resource, QueueVk *dstQueue) : _resource(resource), _srcQueue(_resource->_ownerQueue), _dstQueue(dstQueue) { InitTransition(); }

    void InitTransition()
    {
      std::vector<vk::BufferMemoryBarrier> bufferBarriers;
      std::vector<vk::ImageMemoryBarrier> imageBarriers;
      vk::PipelineStageFlags srcStageFlags;

      _srcCmds = _srcQueue->AllocateCmdBuffer();
      _srcCmds->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

      _dstCmds = _dstQueue->AllocateCmdBuffer();
      _dstCmds->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

      _resource->RecordTransitionCommands(_srcCmds.get(), _srcQueue, _dstCmds.get(), _dstQueue, _dstStageFlags);

      _dstCmds->end();

      _srcCmds->end();

      DeviceVk *device = _dstQueue->_device;
      _srcCmdsDoneSemaphore = device->CreateSemaphore();

      _resource->_ownerQueue = _dstQueue;
    }

    void ExecuteQueueTransition(OperationQueue *operationQueue)
    {
      ASSERT(_resource->_ownerQueue->_family != _dstQueue->_family);

      std::array<vk::SubmitInfo, 1> srcSubmit;
      srcSubmit[0]
        .setCommandBufferCount(1)
        .setPCommandBuffers(&_srcCmds.get())
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(&_srcCmdsDoneSemaphore.get());
      _srcQueue->_queue.submit(srcSubmit, nullptr);

      std::array<vk::SubmitInfo, 1> dstSubmit;
      dstSubmit[0]
        .setCommandBufferCount(1)
        .setPCommandBuffers(&_dstCmds.get())
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(&_srcCmdsDoneSemaphore.get())
        .setPWaitDstStageMask(&_dstStageFlags);
      _dstQueue->_queue.submit(dstSubmit, nullptr);
    }

    vk::UniqueCommandBuffer _srcCmds, _dstCmds;
    vk::UniqueSemaphore _srcCmdsDoneSemaphore;
    vk::PipelineStageFlags _dstStageFlags;
    Resource *_resource;
    QueueVk *_srcQueue, *_dstQueue;
  };

  std::unique_ptr<QueueTransition> GetQueueTransitionTo(QueueVk *queue)
  {
    if (!_ownerQueue) {
      _ownerQueue = queue;
      return nullptr;
    }
    if (_ownerQueue->_family == queue->_family)
      return nullptr;
    Resource *resource = AsResource();
    auto transition = std::make_unique<QueueTransition>(resource, queue);
    return transition;
  }

  // Resource has to have the following method
  // void RecordTransitionCommands(vk::CommandBuffer srcCommands, QueueVk *srcQueue, vk::CommandBuffer dstCommands, QueueVk *dstQueue, vk::PipelineStageFlags &dstStageFlags);

  Resource *AsResource() { return static_cast<Resource*>(this); }

  QueueVk *_ownerQueue = nullptr;
};

NAMESPACE_END(gr)