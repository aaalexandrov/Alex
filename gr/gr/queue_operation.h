#pragma once

#include "util/rtti.h"
#include "graphics_exception.h"

NAMESPACE_BEGIN(gr)

class GraphicsResource;
class OperationQueue;

class QueueOperation : public std::enable_shared_from_this<QueueOperation> {
public:
  virtual void Prepare(OperationQueue *operationQueue) = 0;
  virtual void Execute(OperationQueue *operationQueue) = 0;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::QueueOperation)
