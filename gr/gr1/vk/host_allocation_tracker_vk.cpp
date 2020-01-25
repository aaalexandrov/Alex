#include "host_allocation_tracker_vk.h"
#include "util/mathutl.h"
#include "util/mem.h"
#include <cstdlib>

NAMESPACE_BEGIN(gr1)

static bool IsAligned(void *mem, size_t alignment)
{
  ASSERT(mem != nullptr);
  ASSERT(util::IsPowerOf2(alignment));
  return !(reinterpret_cast<size_t>(mem) & (alignment - 1));
}

VKAPI_ATTR void *VKAPI_CALL HostAllocationTrackerVk::Allocate(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
  HostAllocationTrackerVk *tracker = static_cast<HostAllocationTrackerVk *>(pUserData);

  void *mem = malloc(size);

  ASSERT(IsAligned(mem, alignment));

  ++tracker->_allocations;
  tracker->_sizeAllocated += util::MemSize(mem);

  return mem;
}

VKAPI_ATTR void *VKAPI_CALL HostAllocationTrackerVk::Reallocate(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
  HostAllocationTrackerVk *tracker = static_cast<HostAllocationTrackerVk *>(pUserData);

  size_t prevSize = util::MemSize(pOriginal);

  void *mem = realloc(pOriginal, size);
  ASSERT(IsAligned(mem, alignment));

  ++tracker->_reallocations;
  tracker->_sizeAllocated += util::MemSize(mem) - prevSize;

  return mem;
}

VKAPI_ATTR void VKAPI_CALL HostAllocationTrackerVk::Free(void *pUserData, void *pMemory)
{
  HostAllocationTrackerVk *tracker = static_cast<HostAllocationTrackerVk *>(pUserData);

  ++tracker->_deallocations;
  tracker->_sizeAllocated -= util::MemSize(pMemory);

  free(pMemory);
}

VKAPI_ATTR void VKAPI_CALL HostAllocationTrackerVk::InternalAllocationNotify(void *pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
  HostAllocationTrackerVk *tracker = static_cast<HostAllocationTrackerVk *>(pUserData);

  ++tracker->_internalAllocations;
  tracker->_sizeAllocatedInternal += size;
}

VKAPI_ATTR void VKAPI_CALL HostAllocationTrackerVk::InternalFreeNotify(void *pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
{
  HostAllocationTrackerVk *tracker = static_cast<HostAllocationTrackerVk *>(pUserData);

  ++tracker->_internalDeallocations;
  tracker->_sizeAllocatedInternal -= size;
}

NAMESPACE_END(gr1)