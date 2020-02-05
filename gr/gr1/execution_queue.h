#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"

NAMESPACE_BEGIN(gr1)

class OutputPass;
enum class DependencyType;
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

class Device;
class OutputPass;
enum class DependencyType;
class Resource;
class FinalPass;
enum class ResourceState;
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
	virtual void Execute();

	virtual void ProcessPassDependencies();

	struct ResourceStateData {
		ResourceState _state;
		OutputPass *_outputOfPass = nullptr;
		bool _wasInput = false;
	};

	ResourceStateData &GetResourceStateData(Resource* resource);

	Device *_device;
	std::shared_ptr<FinalPass> _finalPass;

	std::vector<std::shared_ptr<OutputPass>> _passes;
	std::vector<std::shared_ptr<OutputPass>> _dependencyPasses;
	std::vector<OutputPass*> _scheduledPasses;
	std::unordered_map<Resource*, ResourceStateData> _resourceStates;
	PassDependencyTracker _dependencyTracker;
};

NAMESPACE_END(gr1)