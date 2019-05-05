#pragma once

#include "vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

struct MemoryVk {
  vk::UniqueDeviceMemory _memory;
  uint64_t _size;

  MemoryVk(DeviceVk &device, uint64_t size, uint32_t validMemoryTypes, vk::MemoryPropertyFlags memoryFlags);

  void *Map(uint64_t offset, uint64_t size);
  void Unmap();

  vk::Device GetDevice() { return _memory.getOwner(); }

};

NAMESPACE_END(gr)