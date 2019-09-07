#pragma once

#include "util/rtti.h"
#include "graphics_exception.h"

NAMESPACE_BEGIN(gr)

class GraphicsResource;
class OperationQueue;

class QueueOperation : public std::enable_shared_from_this<QueueOperation> {
public:
  enum class Status {
    Outstanding,
    Executing,
    Complete,
  };

  virtual int GetInputResourcesCount() const { return 0; }
  virtual std::shared_ptr<GraphicsResource> &GetInputResource(int index) const { throw GraphicsException("Queue operation input resource index out of range", -1); }

  virtual int GetOutputResourcesCount() const { return 0; }
  virtual std::shared_ptr<GraphicsResource> &GetOutputResource(int index) const { throw GraphicsException("Queue operation output resource index out of range", -1); }

  Status _status = Status::Outstanding;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::QueueOperation)
