#define VMA_IMPLEMENTATION

#include "vma.h"
#include "../graphics_exception.h"

NAMESPACE_BEGIN(gr1)

UniqueVmaAllocator VmaAllocatorCreateUnique(vk::PhysicalDevice physicalDevice, vk::Device device, vk::AllocationCallbacks *allocCallbacks)
{
  VmaAllocatorCreateInfo allocInfo = {};
  allocInfo.physicalDevice = physicalDevice;
  allocInfo.device = device;
  allocInfo.pAllocationCallbacks = reinterpret_cast<VkAllocationCallbacks*>(allocCallbacks);

  VmaAllocator allocator;
  VkResult err = vmaCreateAllocator(&allocInfo, &allocator);
  if (err != VK_SUCCESS)
    throw GraphicsException("vmaAllocatorCreate() failed", err);
  return UniqueVmaAllocator(allocator);
}

UniqueVmaAllocation VmaAllocateMemoryUnique(VmaAllocator allocator, vk::MemoryRequirements const &memReq, vk::MemoryPropertyFlags memPropFlags)
{
  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.memoryTypeBits = memReq.memoryTypeBits;
  allocInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(memPropFlags);

  VmaAllocation allocation;
  VkResult err = vmaAllocateMemory(allocator, &static_cast<VkMemoryRequirements>(memReq), &allocInfo, &allocation, nullptr);
  if (err != VK_SUCCESS)
    throw GraphicsException("vmaAllocateMemory() failed", err);

  return UniqueVmaAllocation(allocation, allocator);
}

UniqueVmaAllocation VmaAllocateMemoryUnique(VmaAllocator allocator, vk::MemoryRequirements const &memReq, bool hostVisible)
{
  vk::MemoryPropertyFlags memProps =
    hostVisible
    ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    : vk::MemoryPropertyFlagBits::eDeviceLocal;

  return std::move(VmaAllocateMemoryUnique(allocator, memReq, memProps));
}

void *VmaMapMemory(VmaAllocator allocator, VmaAllocation memory)
{
  void *mapped;
  VkResult res = vmaMapMemory(allocator, memory, &mapped);
  if (res != VK_SUCCESS)
    throw GraphicsException("VmaMapMemory() failed", res);
  return mapped;
}


NAMESPACE_END(gr1)

