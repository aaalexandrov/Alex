#pragma once

#include "util/namespace.h"
#include "queue_operation.h"

NAMESPACE_BEGIN(gr)

class Graphics;
class PresentationSurface;

class OperationQueue {
public:
  virtual Graphics *GetGraphics() const = 0;

  std::shared_ptr<PresentationSurface> GetPresentationSurface() { return _presentationSurface; }
  void SetPresentationSurface(std::shared_ptr<PresentationSurface> &presentationSurface) { _presentationSurface = presentationSurface; }

  void AddOperation(std::unique_ptr<QueueOperation> operation);
  virtual void ClearOperations();

  void ProcessOperations();

  virtual void PreProcessOperations() {}
  virtual void PostProcessOperations() {}


  std::shared_ptr<PresentationSurface> _presentationSurface;

  std::vector<std::unique_ptr<QueueOperation>> _operations;
};

NAMESPACE_END(gr)