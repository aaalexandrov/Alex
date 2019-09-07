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

  std::unordered_map<GraphicsResource*, QueueOperation*> activeUpdates;
  std::vector<QueueOperation*> waitForOperations;
  for (auto &opPtr : _operations) {
    QueueOperation *operation = opPtr.get();

    waitForOperations.clear();
    int inputResources = operation->GetInputResourcesCount();
    for (int i = 0; i < inputResources; ++i) {
      GraphicsResource *inputResource = operation->GetInputResource(i).get();
      auto updateIt = activeUpdates.find(inputResource);
      if (updateIt == activeUpdates.end())
        continue;
      QueueOperation *updateOp = updateIt->second;
      waitForOperations.push_back(updateOp);
      activeUpdates.erase(updateIt);
    }

    operation->Prepare();
    ExecuteOperation(operation, waitForOperations);
    operation->_status = QueueOperation::Status::Executing;

    int outputResources = operation->GetOutputResourcesCount();
    for (int i = 0; i < outputResources; ++i) {
      GraphicsResource *outputResource = operation->GetOutputResource(i).get();
      activeUpdates.insert({ outputResource, operation });
    }
  }

  PostProcessOperations();
}

NAMESPACE_END(gr)

