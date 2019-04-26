#include "physical_device_vk.h"
#include "util/mathutl.h"
#include <array>

namespace gr {

PhysicalDeviceVk::PhysicalDeviceVk(GraphicsVk *graphics, vk::PhysicalDevice physicalDevice) 
  : _graphics(graphics), _physicalDevice(physicalDevice)
{
  _properties = _physicalDevice.getProperties();
  _features = _physicalDevice.getFeatures();
  _memoryProperties = _physicalDevice.getMemoryProperties();
  _queueFamilies = _physicalDevice.getQueueFamilyProperties();

  _graphicsQueueFamily = GetSuitableQueueFamily(vk::QueueFlagBits::eGraphics);
  _computeQueueFamily = GetSuitableQueueFamily(vk::QueueFlagBits::eCompute);
  _transferQueueFamily = GetSuitableQueueFamily(vk::QueueFlagBits::eTransfer);
  _sparseOpQueueFamily = GetSuitableQueueFamily(vk::QueueFlagBits::eSparseBinding);

  if (_graphicsQueueFamily < 0 || _transferQueueFamily < 0)
    throw GraphicsException("Required queue family support not found", VK_ERROR_FEATURE_NOT_PRESENT);

  _depthFormat = GetDepthFormat();

  if (_depthFormat == vk::Format::eUndefined)
    throw GraphicsException("Valid depth format not found", VK_ERROR_FORMAT_NOT_SUPPORTED);
}

int32_t PhysicalDeviceVk::GetSuitableQueueFamily(vk::QueueFlags flags, std::function<bool(int32_t)> predicate)
{
  int32_t best = -1;
  for (auto i = 0; i < _queueFamilies.size(); ++i) {
    auto &queue = _queueFamilies[i];
    if ((queue.queueFlags & flags) != flags || !predicate(i))
      continue;
    if (best < 0 || util::CountSetBits(static_cast<uint32_t>(queue.queueFlags)) < util::CountSetBits(static_cast<uint32_t>(_queueFamilies[best].queueFlags))) {
      // a queue with less overall capabilities is better, that way we get a more dedicated queue for the function
      best = i;
    }
  }

  return best;
}

vk::Format PhysicalDeviceVk::GetDepthFormat()
{
  std::array<vk::Format, 6> formats = {
    vk::Format::eD24UnormS8Uint,
    vk::Format::eD32SfloatS8Uint,
    vk::Format::eD16UnormS8Uint,
    vk::Format::eD32Sfloat,
    vk::Format::eX8D24UnormPack32,
    vk::Format::eD16Unorm,
  };
  for (auto format: formats) {
    vk::FormatProperties props = _physicalDevice.getFormatProperties(format);
    if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
      return format;
  }
  return vk::Format::eUndefined;
}

}