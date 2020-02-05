#include "execution_queue.h"
#include "device.h"
#include "resource.h"
#include "output_pass.h"

NAMESPACE_BEGIN(gr1)

void PassDependencyTracker::SetPassDependencyCreator(PassDependencyCreateFunc creator)
{
	_passDependencyCreator = creator;
}

void PassDependencyTracker::AddDependency(OutputPass *srcPass, OutputPass *dstPass)
{
	std::unique_ptr<PassDependency> passDependency = _passDependencyCreator(srcPass, dstPass);
	PassDependency *passDep = passDependency.get();
	_dependencies.insert(std::move(passDependency));
	_srcToDependency.insert(std::make_pair(passDep->_srcPass, passDep));
	_dstToDependency.insert(std::make_pair(passDep->_dstPass, passDep));
}

void PassDependencyTracker::Clear()
{
	_srcToDependency.clear();
	_dstToDependency.clear();
	_dependencies.clear();
}

void PassDependencyTracker::ForDependencies(OutputPass *pass, DependencyType dependencyType, std::function<void(PassDependency*)> dependencyFunc)
{
	ASSERT(dependencyType == DependencyType::Input || dependencyType == DependencyType::Output);
	auto &map = dependencyType == DependencyType::Input ? _dstToDependency : _srcToDependency;
	auto range = map.equal_range(pass);
	for (auto it = range.first; it != range.second; ++it) {
		dependencyFunc(it->second);
	}
}

bool PassDependencyTracker::HasDependencies(OutputPass *pass, DependencyType dependencyType)
{
	ASSERT(dependencyType == DependencyType::Input || dependencyType == DependencyType::Output);
	auto &map = dependencyType == DependencyType::Input ? _dstToDependency : _srcToDependency;
	auto range = map.equal_range(pass);
	return range.first != range.second;
}

ExecutionQueue::ExecutionQueue(Device &device) 
	: _device(&device) 
{
	_finalPass = _device->CreateResource<FinalPass>();
}

void ExecutionQueue::EnqueuePass(std::shared_ptr<OutputPass> const &pass)
{
	_passes.push_back(pass);
}

ExecutionQueue::ResourceStateData &ExecutionQueue::GetResourceStateData(Resource *resource)
{
	auto it = _resourceStates.find(resource);
	if (it == _resourceStates.end()) {
		it = _resourceStates.insert(std::make_pair(resource, ResourceStateData{ resource->GetResourceState() })).first;
	}
	return it->second;
}

void ExecutionQueue::ExecutePasses()
{
	EnqueuePass(_finalPass);

	ProcessPassDependencies();

	Prepare();

	Execute();

	WaitExecutionFinished();

	CleanupAfterExecution();
}

void ExecutionQueue::Prepare()
{
	for (uint32_t i = 0; i < _scheduledPasses.size(); ++i) {
		_scheduledPasses[i]->Prepare();
	}
}

void ExecutionQueue::Execute()
{
	for (uint32_t i = 0; i < _scheduledPasses.size(); ++i) {
		_scheduledPasses[i]->Execute(_dependencyTracker);
	}
}


void ExecutionQueue::WaitExecutionFinished()
{
	_finalPass->WaitToFinish();
}

void ExecutionQueue::CleanupAfterExecution()
{
	for (auto resState : _resourceStates) {
		resState.first->SetResourceState(resState.second._state);
	}
	_resourceStates.clear();

	_dependencyTracker.Clear();
	_scheduledPasses.clear();
	_dependencyPasses.clear();
	_passes.clear();
}

void ExecutionQueue::ProcessPassDependencies()
{
	ASSERT(_passes.back() == _finalPass);
	ASSERT(!_resourceStates.size());
	for (int i = 0; i < _passes.size(); ++i) {
		auto &pass = _passes[i];
		DependencyFunc addInputDependency = [&](Resource *resource, ResourceState state) {
			ResourceStateData &recordedData = GetResourceStateData(resource);
			if (recordedData._state != state || recordedData._outputOfPass) {
				bool shouldTransition = recordedData._state != state;
				if (shouldTransition) {
					std::shared_ptr<ResourceStateTransitionPass> transition = resource->CreateTransitionPass(recordedData._state, state);

					if (recordedData._outputOfPass) {
						_dependencyTracker.AddDependency(recordedData._outputOfPass, transition.get());
					}

					_scheduledPasses.push_back(transition.get());

					_dependencyTracker.AddDependency(transition.get(), pass.get());
					recordedData._outputOfPass = transition.get();

					_dependencyPasses.push_back(std::move(transition));
				} else {
					if (recordedData._outputOfPass && !(i == _passes.size() - 1 && recordedData._wasInput)) {
						_dependencyTracker.AddDependency(recordedData._outputOfPass, pass.get());
					}
				}

				recordedData._state = state;
				recordedData._wasInput = true;
			}
		};

		_passes[i]->GetDependencies(DependencyType::Input, addInputDependency);

		DependencyFunc addOutputDependency = [&](Resource *resource, ResourceState state) {
			ResourceStateData &recordedData = GetResourceStateData(resource);
			recordedData._state = state;
			recordedData._outputOfPass = pass.get();
			recordedData._wasInput = false;
		};

		_passes[i]->GetDependencies(DependencyType::Output, addOutputDependency);

		_scheduledPasses.push_back(_passes[i].get());
	}

	// all passes that don't have follow-up passes need to be added as input dependencies of the final pass so it waits for them to finish
	for (int i = 0; i < _scheduledPasses.size() - 1; ++i) {
		auto &pass = _scheduledPasses[i];
		if (_dependencyTracker.HasDependencies(pass, DependencyType::Output))
			continue;
		_dependencyTracker.AddDependency(pass, _finalPass.get());
	}
}

NAMESPACE_END(gr1)

