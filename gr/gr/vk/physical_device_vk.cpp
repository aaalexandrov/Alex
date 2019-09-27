#include "physical_device_vk.h"
#include "presentation_surface_vk.h"
#include "util/mathutl.h"
#include <array>

NAMESPACE_BEGIN(gr)

std::vector<vk::Format> PhysicalDeviceVk::_depthFormats { 
  vk::Format::eD24UnormS8Uint,
  vk::Format::eD32SfloatS8Uint,
  vk::Format::eD16UnormS8Uint,
  vk::Format::eD32Sfloat,
  vk::Format::eX8D24UnormPack32,
  vk::Format::eD16Unorm,
};

PhysicalDeviceVk::PhysicalDeviceVk(GraphicsVk *graphics, vk::PhysicalDevice physicalDevice, PresentationSurfaceVk *initialSurface) 
  : _graphics{ graphics }
  , _physicalDevice{ physicalDevice }
{
  _properties = _physicalDevice.getProperties();
  _features = _physicalDevice.getFeatures();
  _memoryProperties = _physicalDevice.getMemoryProperties();
  _queueFamilies = _physicalDevice.getQueueFamilyProperties();

  QueueFamilyIndex(QueueRole::Graphics) = GetSuitableQueueFamily(vk::QueueFlagBits::eGraphics);
  QueueFamilyIndex(QueueRole::Compute) = GetSuitableQueueFamily(vk::QueueFlagBits::eCompute);
  QueueFamilyIndex(QueueRole::Transfer) = GetSuitableQueueFamily(vk::QueueFlagBits::eTransfer);
  QueueFamilyIndex(QueueRole::SparseOp) = GetSuitableQueueFamily(vk::QueueFlagBits::eSparseBinding);
  QueueFamilyIndex(QueueRole::Present) = GetPresentQueueFamily(initialSurface);

  if (QueueFamilyIndex(QueueRole::Graphics) < 0 || QueueFamilyIndex(QueueRole::Transfer) < 0)
    throw GraphicsException("Required queue family support not found", VK_ERROR_FEATURE_NOT_PRESENT);

  if (QueueFamilyIndex(QueueRole::Present) < 0)
    throw GraphicsException("Present queue family for surface not found", VK_ERROR_FEATURE_NOT_PRESENT);

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

int32_t PhysicalDeviceVk::GetPresentQueueFamily(PresentationSurfaceVk *surface)
{
  int32_t queueFamily = GetSuitableQueueFamily(vk::QueueFlags(), [this, surface](int32_t q) {
    return _physicalDevice.getSurfaceSupportKHR(q, *surface->_surface);
  });
  return queueFamily;
}

vk::SurfaceFormatKHR PhysicalDeviceVk::GetSurfaceFormat(PresentationSurfaceVk *surface)
{
  std::vector<vk::SurfaceFormatKHR> formats = _physicalDevice.getSurfaceFormatsKHR(*surface->_surface);
  auto format = formats[0];
  if (format.format == vk::Format::eUndefined)
    format.format = vk::Format::eR8G8B8A8Unorm;
  return format;
}

vk::PresentModeKHR PhysicalDeviceVk::GetSurfacePresentMode(PresentationSurfaceVk *surface)
{
  vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo;
  std::vector<vk::PresentModeKHR> modes = _physicalDevice.getSurfacePresentModesKHR(*surface->_surface);
  if (std::find(modes.begin(), modes.end(), mode) == modes.end()) {
    ASSERT(!"Fifo present mode should always be available!");
    mode = modes[0];
  }
  return mode;
}

vk::Format PhysicalDeviceVk::GetDepthFormat()
{
  for (auto format: _depthFormats) {
    vk::FormatProperties props = _physicalDevice.getFormatProperties(format);
    if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
      return format;
  }
  return vk::Format::eUndefined;
}

bool PhysicalDeviceVk::IsDepthFormat(vk::Format format)
{
  return std::find(_depthFormats.begin(), _depthFormats.end(), format) != _depthFormats.end();
}

uint32_t PhysicalDeviceVk::GetMemoryTypeIndex(uint32_t validMemoryTypes, vk::MemoryPropertyFlags memoryFlags)
{
  for (uint32_t i = 0; i < _memoryProperties.memoryTypeCount; ++i) {
    if ((validMemoryTypes & (1 << i)) && (_memoryProperties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags)
      return i;
  }
  return -1;
}

NAMESPACE_END(gr)