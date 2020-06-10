#include "host.h"
#include "device.h"
#include "execution_queue.h"

NAMESPACE_BEGIN(gr1)

Host::Host()
{
	for (rttr::type platformType : rttr::type::get<HostPlatform>().get_derived_classes()) {
		rttr::variant hostPlatform = platformType.create();
		_platforms.push_back(hostPlatform.get_value<std::shared_ptr<HostPlatform>>());

		size_t firstInd = _deviceInfos.size();
		_platforms.back()->GetSupportedDevices(_deviceInfos);
		while (firstInd < _deviceInfos.size()) {
			_deviceInfos[firstInd++]._platformType = platformType;
		}
	}
}

uint32_t Host::GetDeviceCount()
{
	return static_cast<uint32_t>(_deviceInfos.size());
}

Host::DeviceInfo Host::GetDeviceInfo(uint32_t deviceIndex)
{
	return _deviceInfos[deviceIndex];
}

std::shared_ptr<Device> Host::CreateDevice(uint32_t deviceIndex, ValidationLevel validation)
{
	if (validation == ValidationLevel::Auto) {
#ifndef NDEBUG
		validation = ValidationLevel::Full;
#else
		validation = ValidationLevel::None;
#endif
	}
	auto itPlatform = std::find_if(_platforms.begin(), _platforms.end(), [&](auto plt) {
		return _deviceInfos[deviceIndex]._platformType == rttr::type::get(*plt);
	});
	return (*itPlatform)->CreateDevice(_deviceInfos[deviceIndex], validation);
}

NAMESPACE_END(gr1)

