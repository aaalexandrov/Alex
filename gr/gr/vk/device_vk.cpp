#include "device_vk.h"

#include "graphics_vk.h"
#include "physical_device_vk.h"

NAMESPACE_BEGIN(gr)

DeviceVk::DeviceVk(PhysicalDeviceVk *physicalDevice)
  : _physicalDevice{ physicalDevice }
{
  _layers = GetPhysicalDevice().enumerateDeviceLayerProperties();
  _extensions = GetPhysicalDevice().enumerateDeviceExtensionProperties();

  std::vector<char const *> layerNames, extensionNames;

  if (GetGraphics()->_validationLevel > Graphics::ValidationLevel::None) {
    GraphicsVk::AppendLayer(layerNames, _layers, "VK_LAYER_LUNARG_standard_validation");
  }

  GraphicsVk::AppendExtension(extensionNames, _extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::vector<vk::DeviceQueueCreateInfo> queuesInfo;
  std::vector<float> queuesPriorities;

  InitQueueFamilies(queuesInfo, queuesPriorities);

  vk::DeviceCreateInfo deviceInfo;
  deviceInfo
    .setQueueCreateInfoCount(static_cast<int32_t>(queuesInfo.size()))
    .setPQueueCreateInfos(queuesInfo.data())
    .setEnabledLayerCount(static_cast<int32_t>(layerNames.size()))
    .setPpEnabledLayerNames(layerNames.data())
    .setEnabledExtensionCount(static_cast<int32_t>(extensionNames.size()))
    .setPpEnabledExtensionNames(extensionNames.data());

  _device = GetPhysicalDevice().createDeviceUnique(deviceInfo, GetGraphics()->AllocationCallbacks());

  _allocator = VmaAllocatorCreateUnique(_physicalDevice->_physicalDevice, *_device, GetGraphics()->AllocationCallbacks());

  InitQueues();
}

void DeviceVk::InitQueueFamilies(std::vector<vk::DeviceQueueCreateInfo>& queuesInfo, std::vector<float> &queuesPriorities)
{
  if (_physicalDevice->_presentQueueFamily < 0)
    throw GraphicsException("Physical device presentation queue family not initialized, you need to create a surface first!", VK_RESULT_MAX_ENUM);

  std::vector<int32_t> families {
    _physicalDevice->_graphicsQueueFamily,
    _physicalDevice->_presentQueueFamily,
    _physicalDevice->_transferQueueFamily,
    _physicalDevice->_computeQueueFamily,
    _physicalDevice->_sparseOpQueueFamily,
  };

  families.erase(std::remove_if(families.begin(), families.end(), [](int32_t f)->bool { return f < 0; }), families.end());

  // reserve priority array so it doesn't get reallocated while we add to it
  queuesPriorities.reserve(families.size());

  while (families.size() > 0) {
    int32_t family = families[0];
    int32_t count = 0;
    auto removeAndCount = [family, &count](int32_t f)->bool {
      if (f != family)
        return false;
      ++count;
      return true;
    };
    families.erase(std::remove_if(families.begin(), families.end(), removeAndCount), families.end());
    // initialize with equal priority of 0
    std::fill_n(std::back_inserter(queuesPriorities), count, 0.0f);

    queuesInfo.emplace_back(vk::DeviceQueueCreateFlags(), family, count, &queuesPriorities[queuesPriorities.size() - count]);
  }
}

void DeviceVk::InitQueues()
{
  std::unordered_map<int32_t, int32_t> usedQueues;
  auto getQueue = [&](QueueVk &queue, int32_t queueFamily, QueueVk::Role role)->void {
    if (queueFamily < 0)
      return;

    auto count = usedQueues[queueFamily]++;
    queue.Init(*this, queueFamily, count, role);
  };

  getQueue(_graphicsQueue, _physicalDevice->_graphicsQueueFamily, QueueVk::Role::Graphics);
  getQueue(_presentQueue, _physicalDevice->_presentQueueFamily, QueueVk::Role::Present);
  getQueue(_transferQueue, _physicalDevice->_transferQueueFamily, QueueVk::Role::Transfer);
  getQueue(_computeQueue, _physicalDevice->_computeQueueFamily, QueueVk::Role::Compute);
  getQueue(_sparseOpQueue, _physicalDevice->_sparseOpQueueFamily, QueueVk::Role::SparseOp);
}

vk::UniqueSemaphore DeviceVk::CreateSemaphore()
{
  return _device->createSemaphoreUnique(vk::SemaphoreCreateInfo(), AllocationCallbacks());
}

void DeviceVk::RenderInstance(std::shared_ptr<ModelInstance> &modelInst)
{
}

GraphicsVk *DeviceVk::GetGraphics()
{
  return _physicalDevice->_graphics;
}

vk::AllocationCallbacks * DeviceVk::AllocationCallbacks()
{
  return GetGraphics()->AllocationCallbacks();
}

vk::PhysicalDevice &DeviceVk::GetPhysicalDevice()
{
  return _physicalDevice->_physicalDevice;
}

NAMESPACE_END(gr)