#pragma once

#include "vk.h"

namespace gr {

struct HostAllocationTrackerVk {
  vk::AllocationCallbacks _allocCallbacks{ this, Allocate, Reallocate, Free, InternalAllocationNotify, InternalFreeNotify };

  size_t _allocations = 0;
  size_t _deallocations = 0;
  size_t _reallocations = 0;
  size_t _internalAllocations = 0;
  size_t _internalDeallocations = 0;

  size_t _sizeAllocated = 0;
  size_t _sizeAllocatedInternal = 0;

  static VKAPI_ATTR void *VKAPI_CALL Allocate(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
  static VKAPI_ATTR void *VKAPI_CALL Reallocate(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
  static VKAPI_ATTR void VKAPI_CALL Free(void *pUserData, void *pMemory);
  static VKAPI_ATTR void VKAPI_CALL InternalAllocationNotify(void *pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
  static VKAPI_ATTR void VKAPI_CALL InternalFreeNotify(void *pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
};

}