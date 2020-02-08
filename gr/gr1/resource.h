#pragma once

#include <memory>
#include "util/namespace.h"
#include "rttr/rttr_enable.h"

NAMESPACE_BEGIN(gr1)

enum class ContentTreatment {
	Keep,
	Clear,
	DontCare,
};

enum class ResourceState {
	Initial = 1 << 0,
	ShaderRead = 1 << 1,
	TransferRead = 1 << 2,
	TransferWrite = 1 << 3,
	RenderWrite = 1 << 4,
	PresentRead = 1 << 5,
	PresentAcquired = 1 << 6,
};

class Device;

class ResourceBase : public std::enable_shared_from_this<ResourceBase> {
	RTTR_ENABLE()
public:
	ResourceBase(Device &device) : _device(&device) {}
	virtual ~ResourceBase() {}

	template<typename ResourceType>
	std::shared_ptr<ResourceType> AsSharedPtr() { return std::static_pointer_cast<ResourceType>(shared_from_this()); }

	template<typename DeviceType>
	DeviceType *GetDevice() { return static_cast<DeviceType*>(_device); }
protected:
	Device *_device;
};

class ResourceStateTransitionPass;
class Resource : public ResourceBase {
	RTTR_ENABLE(ResourceBase)
public:
	Resource(Device &device) : ResourceBase(device) {}

	inline ResourceState GetResourceState() { return _state; }
	inline void SetResourceState(ResourceState state) { _state = state; }
	virtual std::shared_ptr<ResourceStateTransitionPass> CreateTransitionPass(ResourceState srcState, ResourceState dstState) { return nullptr; }

protected:
	ResourceState _state = ResourceState::Initial;
};

NAMESPACE_END(gr1)

