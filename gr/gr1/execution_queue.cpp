#include "execution_queue.h"
#include "resource.h"
#include "output_pass.h"

NAMESPACE_BEGIN(gr1)

void ExecutionQueue::EnqueuePass(std::shared_ptr<OutputPass> const &pass)
{
	_passes.emplace_back(CreatePassData(pass));
	if (_passes.size() > 1) {
		_passes.back()->_previousPassData = _passes[_passes.size() - 2].get();
	}
}

ResourceState &ExecutionQueue::GetResourceState(Resource *resource)
{
	auto it = _resourceStates.find(resource);
	if (it == _resourceStates.end()) {
		it = _resourceStates.insert(std::make_pair(resource, resource->GetResourceState())).first;
	}
	return it->second;
}

void ExecutionQueue::ExecutePasses()
{
	ProcessStateTransitions();

	Prepare();

	Execute();

	WaitExecutionFinished();

	SetResourceStatesAndClear();
	_passes.clear();
}

std::unique_ptr<PassData> ExecutionQueue::CreatePassData(std::shared_ptr<OutputPass> const &pass)
{
	return std::move(std::make_unique<PassData>(pass));
}

void ExecutionQueue::ProcessStateTransitions()
{
	ASSERT(!_resourceStates.size());
	for (int i = 0; i < _passes.size(); ++i) {
		DependencyFunc addInputDependency = [&](Resource *resource, ResourceState state) {
			ResourceState &recordedState = GetResourceState(resource);
			if (recordedState != state) {
				std::shared_ptr<ResourceStateTransitionPass> transition = _passes[i]->_pass->CreateTransitionPass(recordedState, state);
				if (transition) {
					std::unique_ptr<PassData> transitionPassData = CreatePassData(transition);
					transitionPassData->_previousPassData = _passes[i]->_previousPassData;
					_passes[i]->_transitionPasses.push_back(std::move(transitionPassData));
					recordedState = state;
				}
			}
		};

		_passes[i]->_pass->GetDependencies(DependencyType::Input, addInputDependency);

		DependencyFunc addOutputDependency = [&](Resource *resource, ResourceState state) {
			ResourceState &recordedState = GetResourceState(resource);
			recordedState = state;
		};

		_passes[i]->_pass->GetDependencies(DependencyType::Output, addOutputDependency);
	}
}

void ExecutionQueue::SetResourceStatesAndClear()
{
	for (auto resState : _resourceStates) {
		resState.first->SetResourceState(resState.second);
	}
	_resourceStates.clear();
}


NAMESPACE_END(gr1)

