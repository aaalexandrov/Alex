#pragma once

#include <memory>
#include "util/namespace.h"
#include "rttr/rttr_enable.h"

NAMESPACE_BEGIN(gr1)

class Device;
class Resource : public std::enable_shared_from_this<Resource> {
	RTTR_ENABLE()
public:
	Resource(Device &device) : _device(&device) {}
	virtual ~Resource() {}

	virtual bool IsValid() = 0;

	template<typename DeviceType>
	DeviceType *GetDevice() { return static_cast<DeviceType*>(_device); }
protected:
	Device *_device;
};

NAMESPACE_END(gr1)

