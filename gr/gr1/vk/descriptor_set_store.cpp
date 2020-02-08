#include "descriptor_set_store.h"
#include "device_vk.h"
#include "execution_queue_vk.h"
#include "../graphics_exception.h"

NAMESPACE_BEGIN(gr1)

void DescriptorSetStore::Init(DeviceVk &deviceVk, std::vector<vk::DescriptorPoolSize> const &poolSizes, uint32_t maxDescriptorsInPool)
{
	_deviceVk = &deviceVk;
	_poolSizes = poolSizes;
	_maxDescriptorsInPool = maxDescriptorsInPool;

	AllocateDescriptorPool();
}

vk::UniqueDescriptorSet DescriptorSetStore::AllocateDescriptorSet(vk::DescriptorSetLayout layout)
{
	vk::DescriptorSetAllocateInfo setsInfo;
	setsInfo
		.setDescriptorSetCount(1)
		.setPSetLayouts(&layout);

	VkResult result;
	VkDescriptorSet set;

	uint32_t startIndex = _descPoolIndex;
	while (true) {
		setsInfo.setDescriptorPool(_descPools[_descPoolIndex].get());
		result = vkAllocateDescriptorSets(*_deviceVk->_device, reinterpret_cast<const VkDescriptorSetAllocateInfo*>(&setsInfo), &set);
		if (result == VK_SUCCESS)
			break;
		_descPoolIndex = (_descPoolIndex + 1) % _descPools.size();
		if (_descPoolIndex == startIndex) {
			AllocateDescriptorPool();
			setsInfo.setDescriptorPool(_descPools[_descPoolIndex].get());
			result = vkAllocateDescriptorSets(*_deviceVk->_device, reinterpret_cast<const VkDescriptorSetAllocateInfo*>(&setsInfo), &set);
			break;
		}
	}
	if (result != VK_SUCCESS)
		throw GraphicsException("Failed to allocate a descriptor set!", result);

	return vk::UniqueDescriptorSet(set);
}

void DescriptorSetStore::AllocateDescriptorPool()
{
	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(_maxDescriptorsInPool)
		.setPoolSizeCount(static_cast<uint32_t>(_poolSizes.size()))
		.setPPoolSizes(_poolSizes.data());
	vk::UniqueDescriptorPool descPool = _deviceVk->_device->createDescriptorPoolUnique(poolInfo, _deviceVk->AllocationCallbacks());

	_descPoolIndex = static_cast<uint32_t>(_descPools.size());
	_descPools.push_back(std::move(descPool));
}


NAMESPACE_END(gr1)

