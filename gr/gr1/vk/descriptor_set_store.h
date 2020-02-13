#pragma once

#include "util/namespace.h"
#include "vk.h"
#include "rttr/rttr_enable.h"

NAMESPACE_BEGIN(gr1)

class DeviceVk;
class DescriptorSetStore {
	RTTR_ENABLE()
public:
	void Init(DeviceVk &deviceVk, std::vector<vk::DescriptorSetLayoutBinding> const &layoutBindings, uint32_t maxDescriptorsInPool);
	void Init(DeviceVk &deviceVk, std::vector<vk::DescriptorPoolSize> const &poolSizes, uint32_t maxDescriptorsInPool);

	bool IsValid() { return _deviceVk && _poolSizes.size() && _maxDescriptorsInPool; }

	vk::UniqueDescriptorSet AllocateDescriptorSet(vk::DescriptorSetLayout layout);

	static std::vector<vk::DescriptorPoolSize> GetPoolSizes(std::vector<vk::DescriptorSetLayoutBinding> const &layoutBindings, uint32_t maxDescriptorsInPool);

protected:
	void AllocateDescriptorPool();

	DeviceVk *_deviceVk = nullptr;

	std::vector<vk::DescriptorPoolSize> _poolSizes;
	uint32_t _maxDescriptorsInPool = 0;
	
	std::vector<vk::UniqueDescriptorPool> _descPools;
	uint32_t _descPoolIndex = 0;
};

NAMESPACE_END(gr1)
