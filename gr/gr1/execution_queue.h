#pragma once

#include "definitions.h"
#include "rttr/rttr_enable.h"
#include "util/multithread.h"
#include "util/utl.h"
DISABLE_WARNING_PUSH
DISABLE_WARNING_CONVERSION_TO_SMALLER_TYPE
#include "taskflow/taskflow.hpp"
DISABLE_WARNING_POP

NAMESPACE_BEGIN(gr1)

class OutputPass;
class PassDependency {
	RTTR_ENABLE()
public:
	PassDependency(OutputPass *srcPass, OutputPass *dstPass) : _srcPass(srcPass), _dstPass(dstPass) {}
	virtual ~PassDependency() {}

	OutputPass *_srcPass, *_dstPass;
};

struct PassDependencyTracker {
	using PassDependencyCreateFunc = std::function<std::unique_ptr<PassDependency>(OutputPass*, OutputPass*)>;
	void SetPassDependencyCreator(PassDependencyCreateFunc creator);

	void AddDependency(OutputPass *srcPass, OutputPass *dstPass);
	void Clear();

	void ForDependencies(OutputPass *pass, DependencyType dependencyType, std::function<void(PassDependency*)> dependencyFunc);
	bool HasDependencies(OutputPass *pass, DependencyType dependencyType);

private:
	PassDependencyCreateFunc _passDependencyCreator = [](OutputPass *srcPass, OutputPass *dstPass) { return std::make_unique<PassDependency>(srcPass, dstPass); };
	std::unordered_set<std::unique_ptr<PassDependency>> _dependencies;
	std::unordered_multimap<OutputPass*, PassDependency*> _srcToDependency, _dstToDependency;
};

struct PassScheduler {
	void AddPass(OutputPass *pass, util::SharedQueue<OutputPass*> &doneQueue);
	void AddDependency(OutputPass *srcPass, OutputPass *dstPass);
	void Clear();

	tf::Task GetPassTask(OutputPass *pass);

	PassDependencyTracker _passDependencies;
	std::unordered_map<OutputPass*, tf::Task> _pass2Task;
	tf::Taskflow _prepareTasks;
};

class Device;
class OutputPass;
class Resource;
class FinalPass;
class ExecutionQueue {
	RTTR_ENABLE()
public:
	ExecutionQueue(Device &device);
	virtual ~ExecutionQueue() {}

	virtual void EnqueuePass(std::shared_ptr<OutputPass> const &pass);
	virtual void ExecutePasses();

	template<typename DeviceType>
	DeviceType *GetDevice() { return static_cast<DeviceType*>(_device); }
protected:
	virtual void WaitExecutionFinished();
	virtual void CleanupAfterExecution();
	virtual void Prepare();
	virtual void Submit();

	void ProcessPassDependencies(OutputPass *pass);
	virtual void ProcessPassesDependencies();

	struct ResourceStateData {
		ResourceState _state;
		OutputPass *_outputOfPass = nullptr;
	};

	ResourceStateData &GetResourceStateData(Resource* resource);

	Device *_device;
	std::shared_ptr<FinalPass> _finalPass;
	tf::Executor _taskExecutor;

	bool _executing = false;
	util::SharedQueue<OutputPass*> _submitQueue;
	PassScheduler _passScheduler;

	std::vector<std::shared_ptr<OutputPass>> _passes;
	std::vector<std::shared_ptr<OutputPass>> _dependencyPasses;
	std::unordered_map<Resource*, ResourceStateData> _resourceStates;
};

NAMESPACE_END(gr1)