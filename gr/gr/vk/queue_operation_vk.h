#pragma once

#include "../queue_operation.h"
#include "vk.h"

NAMESPACE_BEGIN(gr)

struct QueueVk;

class QueueOperationVk : public QueueOperation {
public:


  QueueVk *_queue = nullptr;
  vk::UniqueCommandBuffer _commands;
  vk::UniqueSemaphore _semaphoreDone;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::QueueOperationVk, gr::QueueOperation)