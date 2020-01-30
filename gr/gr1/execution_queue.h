#pragma once

#include "util/namespace.h"
#include "rttr/rttr_enable.h"

NAMESPACE_BEGIN(gr1)

class OutputPass;
class PassData {
	RTTR_ENABLE()
public:
	PassData(std::shared_ptr<OutputPass> const &pass) : _pass(pass) {}
	virtual ~PassData() {}

	std::shared_ptr<OutputPass> _pass;
	std::vector<std::unique_ptr<PassData>> _transitionPasses;
	PassData *_previousPassData = nullptr;
};

class Device;
class OutputPass;
class Resource;
enum class ResourceState;
class ExecutionQueue {
	RTTR_ENABLE()
public:
	ExecutionQueue(Device &device) : _device(&device) {}
	virtual ~ExecutionQueue() {}

	virtual void EnqueuePass(std::shared_ptr<OutputPass> const &pass);
	virtual void ExecutePasses();

	template<typename DeviceType>
	DeviceType *GetDevice() { return static_cast<DeviceType*>(_device); }
protected:
	virtual std::unique_ptr<PassData> CreatePassData(std::shared_ptr<OutputPass> const &pass);
	virtual void WaitExecutionFinished() = 0;
	virtual void Prepare() = 0;
	virtual void Execute() = 0;

	virtual void ProcessStateTransitions();
	virtual void SetResourceStatesAndClear();
	 
	ResourceState &GetResourceState(Resource* resource);

	Device *_device;
	std::vector<std::unique_ptr<PassData>> _passes;
	std::unordered_map<Resource*, ResourceState> _resourceStates;
};

NAMESPACE_END(gr1)