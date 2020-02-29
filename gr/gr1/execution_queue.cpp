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

void PassScheduler::AddPass(OutputPass *pass, util::SharedQueue<OutputPass*> &doneQueue)
{
	tf::Task task = GetPassTask(pass);
	task.work([pass, queue=&doneQueue] {
		pass->Prepare();
		queue->PushBack(std::move(const_cast<OutputPass*>(pass)));
	});
}

void PassScheduler::AddDependency(OutputPass *srcPass, OutputPass *dstPass)
{
	_passDependencies.AddDependency(srcPass, dstPass);

	tf::Task srcTask = GetPassTask(srcPass);
	tf::Task dstTask = GetPassTask(dstPass);
	srcTask.precede(dstTask);
}

void PassScheduler::Clear()
{
	_passDependencies.Clear();
	_pass2Task.clear();
	_prepareTasks.clear();
}

tf::Task PassScheduler::GetPassTask(OutputPass *pass)
{
	auto it = _pass2Task.find(pass);
	if (it == _pass2Task.end()) {
		it = _pass2Task.insert(std::make_pair(pass, _prepareTasks.placeholder())).first;
	}
	return it->second;
}


ExecutionQueue::ExecutionQueue(Device &device) 
	: _device(&device) 
{
	_finalPass = _device->CreateResource<FinalPass>();
}

void ExecutionQueue::EnqueuePass(std::shared_ptr<OutputPass> const &pass)
{
	ASSERT(!_executing);
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

	_executing = true;
	ProcessPassDependencies();

	Prepare();

	Submit();

	WaitExecutionFinished();

	CleanupAfterExecution();
	_executing = false;
}

void ExecutionQueue::Prepare()
{
	_taskExecutor.run(_passScheduler._prepareTasks);
}

void ExecutionQueue::Submit()
{
	while (true) {
		OutputPass *pass = _submitQueue.PopFront();
		pass->Submit(_passScheduler._passDependencies);
		if (pass == _finalPass.get())
			break;
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

	ASSERT(!_submitQueue.GetSize());
	_passScheduler.Clear();
	_dependencyPasses.clear();
	_passes.clear();
}

void ExecutionQueue::ProcessInputDependency(Resource *resource, ResourceState state, OutputPass *pass, std::unordered_set<Resource*> &resourcesInTransition)
{
	ResourceStateData &recordedData = GetResourceStateData(resource);
	if (recordedData._state != state || recordedData._outputOfPass) {
		bool shouldTransition = recordedData._state != state;
		if (shouldTransition) {
#if defined(_DEBUG)
			ASSERT(resourcesInTransition.find(resource) == resourcesInTransition.end() && "Circular dependency in transitions!");
			resourcesInTransition.insert(resource);
#endif

			std::shared_ptr<ResourceStateTransitionPass> transition = resource->CreateTransitionPass(recordedData._state, state);

			DependencyFunc addTransitionDep = [&](Resource *resource, ResourceState state) { 
				ProcessInputDependency(resource, state, transition.get(), resourcesInTransition); 
			};
			transition->GetDependencies(DependencyType::Input, addTransitionDep);

			if (recordedData._outputOfPass) {
				_passScheduler.AddDependency(recordedData._outputOfPass, transition.get());
			}

			_passScheduler.AddPass(transition.get(), _submitQueue);

			_passScheduler.AddDependency(transition.get(), pass);
			recordedData._outputOfPass = transition.get();

			_dependencyPasses.push_back(std::move(transition));

#if defined(_DEBUG)
			resourcesInTransition.erase(resource);
#endif
		} else {
			if (recordedData._outputOfPass) {
				_passScheduler.AddDependency(recordedData._outputOfPass, pass);
			}
		}

		recordedData._state = state;
	}
}

void ExecutionQueue::ProcessPassDependencies()
{
	ASSERT(_passes.back() == _finalPass);
	ASSERT(!_resourceStates.size());
	std::unordered_set<Resource*> resourcesInTransition;
	for (int i = 0; i < _passes.size(); ++i) {
		OutputPass *pass = _passes[i].get();
		DependencyFunc addInputDependency = [&](Resource *resource, ResourceState state) { 
			ProcessInputDependency(resource, state, pass, resourcesInTransition); 
		};

		pass->GetDependencies(DependencyType::Input, addInputDependency);

		DependencyFunc addOutputDependency = [&](Resource *resource, ResourceState state) {
			ResourceStateData &recordedData = GetResourceStateData(resource);
			recordedData._state = state;
			recordedData._outputOfPass = pass;
		};

		pass->GetDependencies(DependencyType::Output, addOutputDependency);

		_passScheduler.AddPass(pass, _submitQueue);
	}

	// all passes that don't have follow-up passes need to be added as input dependencies of the final pass so it waits for them to finish
	// we only traverse user passes because transition passes always have a follow-up
	for (int i = 0; i < _passes.size() - 1; ++i) {
		auto pass = _passes[i].get();
		if (_passScheduler._passDependencies.HasDependencies(pass, DependencyType::Output))
			continue;
		_passScheduler.AddDependency(pass, _finalPass.get());
	}
}

NAMESPACE_END(gr1)

