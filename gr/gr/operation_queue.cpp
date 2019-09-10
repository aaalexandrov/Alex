#include "operation_queue.h"
#include "graphics_resource.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr)

void OperationQueue::AddOperation(std::unique_ptr<QueueOperation> operation)
{
  _operations.emplace_back(std::move(operation));
}

void OperationQueue::ClearOperations()
{
  _operations.clear();
}

void OperationQueue::ProcessOperations()
{
  PreProcessOperations();

  for (auto &opPtr : _operations) {
    QueueOperation *operation = opPtr.get();

    operation->Prepare(this);
    operation->Execute(this);
  }

  PostProcessOperations();
}

NAMESPACE_END(gr)

