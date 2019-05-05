#include "memory_vk.h"
#include "device_vk.h"
#include "physical_device_vk.h"
#include "graphics_vk.h"

NAMESPACE_BEGIN(gr)

MemoryVk::MemoryVk(DeviceVk &device, uint64_t size, uint32_t validMemoryTypes, vk::MemoryPropertyFlags memoryFlags)
{
  vk::MemoryAllocateInfo memInfo(size, device._physicalDevice->GetMemoryTypeIndex(validMemoryTypes, memoryFlags));
  _memory = device._device->allocateMemoryUnique(memInfo, device.GetGraphics()->AllocationCallbacks());
  _size = size;
}

void *MemoryVk::Map(uint64_t offset, uint64_t size)
{
  return GetDevice().mapMemory(*_memory, offset, size, vk::MemoryMapFlags());
}

void MemoryVk::Unmap()
{
  GetDevice().unmapMemory(*_memory);
}

NAMESPACE_END(gr)
