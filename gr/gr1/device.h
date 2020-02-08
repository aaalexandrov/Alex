#pragma once

#include "util/namespace.h"
#include "rttr_factory.h"
#include "rttr/rttr_enable.h"
#include "host.h"

NAMESPACE_BEGIN(gr1)

class ResourceBase;
enum class ResourceState;
class OutputPass;
class ExecutionQueue;

class Device {
	RTTR_ENABLE()
public:
	Device(Host::DeviceInfo const &deviceInfo, ValidationLevel validation, rttr::type thisType) 
		: _deviceInfo(deviceInfo), _validationLevel(validation), _resourceFactory(RttrDiscriminator::Tag, thisType) {}
	virtual ~Device() {}

	virtual std::shared_ptr<ResourceBase> CreateResource(rttr::type resourceType);

	template<typename ResourceType>
	std::shared_ptr<ResourceType> CreateResource() { return std::static_pointer_cast<ResourceType>(CreateResource(rttr::type::get<ResourceType>())); }

	inline ExecutionQueue &GetExecutionQueue() { return *_queue.get(); }
	inline Host::DeviceInfo const &GetDeviceInfo() const { return _deviceInfo; }
	inline ValidationLevel GetValidationLevel() const { return _validationLevel; }

	inline rttr::type GetResourceDiscriminatorType() { return _resourceFactory._discriminatorValue.get_value<rttr::type>(); }
protected:
	ValidationLevel _validationLevel;
	Host::DeviceInfo _deviceInfo;
	std::unique_ptr<ExecutionQueue> _queue;
	RttrFactory _resourceFactory;
};

NAMESPACE_END(gr1)

