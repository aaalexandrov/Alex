#include "graphics_vk.h"
#include "physical_device_vk.h"
#include "host_allocation_tracker_vk.h"
#include "presentation_surface_vk.h"
#include "device_vk.h"
#include "shader_vk.h"
#include "image_vk.h"
#include "buffer_vk.h"
#include "material_vk.h"
#include "util/dbg.h"

NAMESPACE_BEGIN(gr)

GraphicsVk::GraphicsVk()
{
}

GraphicsVk::~GraphicsVk()
{
}

void GraphicsVk::Init(PresentationSurfaceCreateData &surfaceData)
{
  InitInstance();
  auto surface = CreatePresentationSurface(surfaceData);
  auto surfaceVk = static_cast<PresentationSurfaceVk*>(surface.get());
  InitPhysicalDevice(surfaceVk);
  _device = std::make_unique<DeviceVk>(&*_physicalDevice);
  _renderQueue = std::make_unique<RenderQueueVk>(*_device);
  _renderQueue->SetPresentationSurface(surface);
  surfaceVk->InitForDevice(&*_device);
}

std::shared_ptr<PresentationSurface> GraphicsVk::CreatePresentationSurface(PresentationSurfaceCreateData &createData)
{
  auto surface = std::make_shared<PresentationSurfaceVk>(this, createData);
  if (_device)
    surface->InitForDevice(&*_device);
  return surface;
}

std::shared_ptr<Buffer> GraphicsVk::CreateBuffer(Buffer::Usage usage, BufferDescPtr &description, size_t size)
{
  auto buffer = std::make_shared<BufferVk>(*_device, size, description, usage);
  return buffer;
}

std::shared_ptr<Image> GraphicsVk::CreateImage(Image::Usage usage, ColorFormat format, glm::u32vec3 size, uint32_t mipLevels, uint32_t arrayLayers)
{
  vk::Extent3D ext { size.x, size.y, size.z };
  vk::Format vkFormat = ImageVk::ColorFormat2vk(format);
  auto image = std::make_shared<ImageVk>(*_device, vkFormat, ext, mipLevels, arrayLayers, usage);
  return image;
}

std::shared_ptr<Material> GraphicsVk::CreateMaterial(std::shared_ptr<Shader> &shader)
{
  auto material = std::make_shared<MaterialVk>(*_device, shader);
  return std::shared_ptr<Material>();
}

std::shared_ptr<Shader> GraphicsVk::LoadShader(std::string const &name)
{
  auto shader = std::make_shared<ShaderVk>(*_device, name);
  return shader;
}

void GraphicsVk::InitInstance()
{
  _apiVersion = Vk2Version(vk::enumerateInstanceVersion());
  _instanceLayers = vk::enumerateInstanceLayerProperties();
  _instanceExtensions = vk::enumerateInstanceExtensionProperties();

  std::vector<char const*> layerNames;
  std::vector<char const *> extensionNames;
  if (_validationLevel > ValidationLevel::None) {
    AppendLayer(layerNames, _instanceLayers, "VK_LAYER_LUNARG_standard_validation");
    AppendExtension(extensionNames, _instanceExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  AppendExtension(extensionNames, _instanceExtensions, VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
  AppendExtension(extensionNames, _instanceExtensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(linux)
  AppendExtension(extensionNames, _instanceExtensions, VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
#error Unsupported platform!
#endif

  vk::ApplicationInfo appInfo(_appName.c_str(), Version2Vk(_appVersion), _engineName.c_str(), Version2Vk(_engineVersion));
  vk::InstanceCreateInfo instanceInfo(vk::InstanceCreateFlags(), &appInfo,
    static_cast<uint32_t>(layerNames.size()), layerNames.data(),
    static_cast<uint32_t>(extensionNames.size()), extensionNames.data());

  vk::DebugReportCallbackCreateInfoEXT debugCBInfo(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError,
    DbgReportFunc, this);

  if (_validationLevel > ValidationLevel::None) {
    instanceInfo.pNext = &debugCBInfo;

    _hostAllocTracker = std::make_unique<HostAllocationTrackerVk>();
  }

  _instance = vk::createInstanceUnique(instanceInfo);

  _dynamicDispatch.init(*_instance);

  if (_validationLevel > ValidationLevel::None) {
    _debugReportCallback = _instance->createDebugReportCallbackEXTUnique(debugCBInfo, AllocationCallbacks(), _dynamicDispatch);
  }
}

void GraphicsVk::InitPhysicalDevice(PresentationSurfaceVk *initialSurface)
{
  std::vector<vk::PhysicalDevice> physicalDevices = _instance->enumeratePhysicalDevices();
  _physicalDevice = std::make_unique<PhysicalDeviceVk>(this, physicalDevices[0], initialSurface);
}

vk::LayerProperties const *GraphicsVk::GetLayer(std::vector<vk::LayerProperties> const &layers, std::string const &layerName)
{
  auto layerIt = std::find_if(layers.begin(), layers.end(), 
    [&](vk::LayerProperties const& layer)->bool { return layerName == layer.layerName; });
  if (layerIt == layers.end())
    return nullptr;
  return &*layerIt;
}

vk::ExtensionProperties const *GraphicsVk::GetExtension(std::vector<vk::ExtensionProperties> const &extensions, std::string const &extensionName)
{
  auto extIt = std::find_if(extensions.begin(), extensions.end(),
    [&](vk::ExtensionProperties const& extension)->bool { return extensionName == extension.extensionName; });
  if (extIt == extensions.end())
    return nullptr;
  return &*extIt;
}

void GraphicsVk::AppendLayer(std::vector<char const *> &layers, std::vector<vk::LayerProperties> const &availableLayers, std::string const &layerName)
{
  auto layer = GetLayer(availableLayers, layerName);
  if (!layer)
    throw GraphicsException("GraphicsVk::AppendLayer() failed to find layer " + layerName, VK_RESULT_MAX_ENUM);
  layers.push_back(layer->layerName);
}

void GraphicsVk::AppendExtension(std::vector<char const *> &extensions, std::vector<vk::ExtensionProperties> const &availableExtensions, std::string const &extensionName)
{
  auto ext = GetExtension(availableExtensions, extensionName);
  if (!ext)
    throw GraphicsException("GraphicsVk::AppendExtension() failed to find extension " + extensionName, VK_RESULT_MAX_ENUM);
  extensions.push_back(ext->extensionName);
}

vk::AllocationCallbacks *GraphicsVk::AllocationCallbacks()
{
  if (!_hostAllocTracker)
    return nullptr;
  return &_hostAllocTracker->_allocCallbacks;
}

VKAPI_ATTR VkBool32 VKAPI_CALL GraphicsVk::DbgReportFunc(
  VkFlags msgFlags, 
  VkDebugReportObjectTypeEXT objType, 
  uint64_t srcObject, 
  size_t location, 
  int32_t msgCode, 
  const char *pLayerPrefix, 
  const char *pMsg, 
  void *pUserData)
{
  std::string flag = (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ? "error" : "warning";

  LOG("Vulkan debug ", flag, ", code: ", msgCode, ", layer: ", pLayerPrefix, ", msg: ", pMsg);

  return false;
}

NAMESPACE_END(gr)