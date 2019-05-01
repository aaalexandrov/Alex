#pragma once

#include "../graphics.h"
#include "vk.h"

namespace gr {

class PhysicalDeviceVk;
struct HostAllocationTrackerVk;
class PresentationSurfaceVk;
class DeviceVk;

class GraphicsVk : public Graphics {
public:

  GraphicsVk();
  ~GraphicsVk() override;

  void Init(PresentationSurfaceCreateData &surfaceData) override;

  PresentationSurface *CreatePresentationSurface(PresentationSurfaceCreateData &createData) override;

  PresentationSurface *GetDefaultPresentationSurface() override;

  void InitInstance();
  void InitPhysicalDevice();

  static uint32_t Version2Vk(Version const &version) { return VK_MAKE_VERSION(version._major, version._minor, version._patch); }
  static Version  Vk2Version(uint32_t version) { return Version{ VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version) }; }

  static vk::LayerProperties const *GetLayer(std::vector<vk::LayerProperties> const &layers, std::string const &layerName);
  static vk::ExtensionProperties const *GetExtension(std::vector<vk::ExtensionProperties> const &extensions, std::string const &extensionName);

  static void AppendLayer(std::vector<char const *> &layers, std::vector<vk::LayerProperties> const &availableLayers, std::string const &layerName);
  static void AppendExtension(std::vector<char const *> &extensions, std::vector<vk::ExtensionProperties> const &availableExtensions, std::string const &extensionName);

  vk::AllocationCallbacks *AllocationCallbacks();

  static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);

  std::string _engineName{ "GR_VK" };
  Version _engineVersion{ 0, 1, 0 };

  std::vector<vk::LayerProperties> _instanceLayers;
  std::vector<vk::ExtensionProperties> _instanceExtensions;

  // Allocation tracker needs to appear before any Vulkan RAII resources
  // Resources will be calling it during destruction, so the tracker has to outlive them
  std::unique_ptr<HostAllocationTrackerVk> _hostAllocTracker;

  Version _apiVersion;
  vk::UniqueInstance _instance;
  vk::DispatchLoaderDynamic _dynamicDispatch;

  using UniqueDebugReportCallbackEXT = vk::UniqueHandle<vk::DebugReportCallbackEXT, vk::DispatchLoaderDynamic>;
  UniqueDebugReportCallbackEXT _debugReportCallback;

  std::unique_ptr<PhysicalDeviceVk> _physicalDevice;

  std::unique_ptr<PresentationSurfaceVk> _presentationSurface;

  std::unique_ptr<DeviceVk> _device;
};

}