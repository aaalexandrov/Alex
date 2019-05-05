#pragma once

#include "util/dbg.h"
#include "util/namespace.h"
#include "util/mem.h"

#define VMA_ASSERT(cond) ASSERT(cond)

#include "vk.h"
#include "vk_mem_alloc/vk_mem_alloc.h"

NAMESPACE_BEGIN(gr)

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

NAMESPACE_END(gr)