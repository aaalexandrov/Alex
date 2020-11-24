#pragma once

#include <string>
#include "util/namespace.h"
#include "rttr/rttr_enable.h"

NAMESPACE_BEGIN(gr1)

class Device;
class HostPlatform;

enum class ValidationLevel {
	None,
	Auto,
	Full,
};

enum class RttrDiscriminator { Tag };

struct PresentationSurfaceCreateData;
class Host {
	RTTR_ENABLE()
public:
	struct DeviceInfo {
		std::string _name;
		glm::uvec3 _version;
		rttr::type _platformType = rttr::type::get<void>();
	};

	Host();

	uint32_t GetDeviceCount();
	DeviceInfo GetDeviceInfo(uint32_t deviceIndex);

	std::shared_ptr<Device> CreateDevice(uint32_t deviceIndex, PresentationSurfaceCreateData const *surfaceData, ValidationLevel validation = ValidationLevel::Auto);

private:
	std::vector<DeviceInfo> _deviceInfos;
	std::vector<std::shared_ptr<HostPlatform>> _platforms;
};

class HostPlatform {
	RTTR_ENABLE()
public:
	virtual ~HostPlatform() {}

	virtual void GetSupportedDevices(std::vector<Host::DeviceInfo> &deviceInfos) = 0;
	virtual std::shared_ptr<Device> CreateDevice(Host::DeviceInfo const &deviceInfo, PresentationSurfaceCreateData const *surfaceData, ValidationLevel validation) = 0;
};

NAMESPACE_END(gr1)