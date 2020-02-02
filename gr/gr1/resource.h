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
	Initial,
	ShaderRead,
	TransferRead,
	TransferWrite,
	RenderWrite,
	PresentRead,
	PresentAcquired,
};

class Device;
class ResourceStateTransitionPass;
class Resource : public std::enable_shared_from_this<Resource> {
	RTTR_ENABLE()
public:
	Resource(Device &device) : _device(&device) {}
	virtual ~Resource() {}

	inline ResourceState GetResourceState() { return _state; }
	inline void SetResourceState(ResourceState state) { _state = state; }
	virtual std::shared_ptr<ResourceStateTransitionPass> CreateTransitionPass(ResourceState srcState, ResourceState dstState) { return nullptr; }

	template<typename DeviceType>
	DeviceType *GetDevice() { return static_cast<DeviceType*>(_device); }
protected:

	Device *_device;
	ResourceState _state = ResourceState::Initial;
};

NAMESPACE_END(gr1)

