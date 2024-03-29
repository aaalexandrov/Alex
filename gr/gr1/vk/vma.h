#pragma once

#include "util/dbg.h"
#include "util/namespace.h"
#include "util/mem.h"

#include "vk.h"

#include "vma/vk_mem_alloc.h"
#ifdef LoadImage
#undef LoadImage
#endif
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
#ifdef CreateWindow
#undef CreateWindow
#endif

NAMESPACE_BEGIN(gr1)

struct VmaAllocatorTraits {
  using Handle = VmaAllocator;
  using Owner = char;

  static constexpr Handle NullHandle() { return nullptr; }
  static constexpr Owner NullOwner() { return 0; }
  static void Destroy(Owner, Handle alloc) { vmaDestroyAllocator(alloc); }
};
using UniqueVmaAllocator = util::UniqueHandle<VmaAllocatorTraits>;

UniqueVmaAllocator VmaAllocatorCreateUnique(vk::PhysicalDevice physicalDevice, vk::Device device, vk::AllocationCallbacks *allocCallbacks);

struct VmaAllocationTraits {
  using Handle = VmaAllocation;
  using Owner = VmaAllocator;

  static constexpr Handle NullHandle() { return nullptr; }
  static constexpr Owner NullOwner() { return nullptr; }
  static void Destroy(Owner allocator, Handle allocation) { vmaFreeMemory(allocator, allocation); }
};

using UniqueVmaAllocation = util::UniqueHandle<VmaAllocationTraits>;

UniqueVmaAllocation VmaAllocateMemoryUnique(VmaAllocator allocator, vk::MemoryRequirements const &memReq, vk::MemoryPropertyFlags memPropFlags);
UniqueVmaAllocation VmaAllocateMemoryUnique(VmaAllocator allocator, vk::MemoryRequirements const &memReq, bool hostVisible);

void *VmaMapMemory(VmaAllocator allocator, VmaAllocation memory);

NAMESPACE_END(gr1)