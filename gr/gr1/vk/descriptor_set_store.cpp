#include "descriptor_set_store.h"
#include "device_vk.h"
#include "execution_queue_vk.h"
#include "../graphics_exception.h"

NAMESPACE_BEGIN(gr1)

DescriptorSetVk::~DescriptorSetVk()
{
}

void DescriptorSetStore::Init(DeviceVk &deviceVk, std::vector<vk::DescriptorSetLayoutBinding> const &layoutBindings, uint32_t maxDescriptorsInPool)
{
	Init(deviceVk, GetPoolSizes(layoutBindings, maxDescriptorsInPool), maxDescriptorsInPool);
}

void DescriptorSetStore::Init(DeviceVk &deviceVk, std::vector<vk::DescriptorPoolSize> const &poolSizes, uint32_t maxDescriptorsInPool)
{
	_deviceVk = &deviceVk;
	_poolSizes = poolSizes;
	_maxDescriptorsInPool = maxDescriptorsInPool;

	AllocateDescriptorPool();
}

DescriptorSetVk DescriptorSetStore::AllocateDescriptorSet(vk::DescriptorSetLayout layout)
{
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	vk::DescriptorSetAllocateInfo setsInfo;
	setsInfo
		.setDescriptorSetCount(1)
		.setPSetLayouts(&layout);

	uint32_t startIndex = _descPoolIndex;
	bool allocatedPool = false;

	while (true) {
		setsInfo.setDescriptorPool(_descPools[_descPoolIndex].get());

		try {
			return DescriptorSetVk(this, std::move(_deviceVk->_device->allocateDescriptorSetsUnique(setsInfo)[0]));
		} catch (vk::SystemError &e) {
			if (allocatedPool) {
				throw GraphicsException("Failed to allocate a descriptor set!", e.code().value());
			}
		}

		_descPoolIndex = (_descPoolIndex + 1) % _descPools.size();
		if (_descPoolIndex == startIndex) {
			AllocateDescriptorPool();
			allocatedPool = true;
		}
	}
}

std::vector<vk::DescriptorPoolSize> DescriptorSetStore::GetPoolSizes(
	std::vector<vk::DescriptorSetLayoutBinding> const &layoutBindings, uint32_t maxDescriptorsInPool)
{
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (auto &binding : layoutBindings) {
		auto it = std::find_if(poolSizes.begin(), poolSizes.end(), [&](auto &poolSize) {
			return poolSize.type == binding.descriptorType;
		});
		if (it == poolSizes.end()) {
			poolSizes.emplace_back(binding.descriptorType);
			it = poolSizes.end() - 1;
		}
		it->setDescriptorCount(it->descriptorCount + maxDescriptorsInPool * binding.descriptorCount);
	}
	return poolSizes;
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

