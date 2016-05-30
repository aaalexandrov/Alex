#include "stdafx.h"
#include "VGraphics.h"
#include "App.h"
#include "Wnd.h"

#include <iostream>
#include <functional>
#include <algorithm>

VGraphics::VGraphics(bool validate) :
  m_validate(validate)
{
}

VGraphics::~VGraphics()
{
}

bool VGraphics::Init()
{
  if (!InitInstance())
    return false;
  if (!InitPhysicalDevice())
    return false;
  if (!InitSwapchain())
    return false;
  if (!InitDevice())
    return false;

  return true;
}

void VGraphics::Done()
{
  DoneDevice();
  DoneSwapchain();
  DonePhysicalDevice();
  DoneInstance();
}

bool VGraphics::AddLayerName(std::vector<VkLayerProperties> const &layers, std::string const &name, std::vector<std::string> &layerNames)
{
  if (std::find(layerNames.begin(), layerNames.end(), name) != layerNames.end())
    return true;
  for (auto &l : layers)
    if (l.layerName == name) {
      layerNames.push_back(name);
      return true;
    }
  std::cerr << "VGraphics: failed to add instance layer " << name << std::endl;
  return false;
}

bool VGraphics::AddExtensionName(std::vector<VkExtensionProperties> const &extensions, std::string const &name, std::vector<std::string> &extNames)
{
  if (std::find(extNames.begin(), extNames.end(), name) != extNames.end())
    return true;
  for (auto &e : extensions)
    if (e.extensionName == name) {
      extNames.push_back(name);
      return true;
    }
  std::cerr << "VGraphics: failed to add instance extension " << name << std::endl;
  return false;
}

bool VGraphics::AppendExtensionProperties(std::vector<VkExtensionProperties> &extensions, std::string const &layerName)
{
  uint32_t extCount;
  if (vkEnumerateInstanceExtensionProperties(layerName.c_str(), &extCount, nullptr) < 0)
    return false;
  size_t oldSize = extensions.size();
  extensions.resize(oldSize + extCount);
  if (vkEnumerateInstanceExtensionProperties(layerName.c_str(), &extCount, &extensions[oldSize]) < 0)
    return false;
  return true;
}

std::vector<char const*> VGraphics::GetPtrArray(std::vector<std::string> const &strs)
{
  std::vector<char const*> ptrs;
  for (auto &s : strs)
    ptrs.push_back(s.c_str());
  return ptrs;
}

template <class T>
VkResult AppendData(std::vector<T> &data, std::function<VkResult(uint32_t&, T*)> getter)
{
  uint32_t count;
  VkResult err = getter(count, nullptr);
  if (err < 0 || !count)
    return err;
  size_t oldSize = data.size();
  data.resize(oldSize + count);
  err = getter(count, &data[oldSize]);
  return err;
}

bool VGraphics::InitInstance()
{
  VkResult err;
  std::vector<VkLayerProperties> layerProps;
  err = AppendData<VkLayerProperties>(layerProps, [](uint32_t &s, VkLayerProperties *p)->VkResult { return vkEnumerateInstanceLayerProperties(&s, p); });
  if (err < 0)
    return false;

  m_instanceLayerNames.clear();
  if (m_validate) {
    if (!AddLayerName(layerProps, "VK_LAYER_LUNARG_standard_validation", m_instanceLayerNames))
      return false;
  }

  std::vector<VkExtensionProperties> extProps;
  err = AppendData<VkExtensionProperties>(extProps, [](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateInstanceExtensionProperties(nullptr, &s, p); });
  if (err < 0)
    return false;
  for (auto &l : m_instanceLayerNames) {
    err = AppendData<VkExtensionProperties>(extProps, [&l](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateInstanceExtensionProperties(l.c_str(), &s, p); });
    if (err < 0)
      return false;
  }
  
  m_instanceExtensionNames.clear();
  if (!AddExtensionName(extProps, VK_KHR_SURFACE_EXTENSION_NAME, m_instanceExtensionNames))
    return false;

#if defined(_WIN32)
  if (!AddExtensionName(extProps, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, m_instanceExtensionNames))
    return false;
#elif defined(linux)
  if (!AddExtensionName(extProps, VK_KHR_XCB_SURFACE_EXTENSION_NAME, extNames))
    return false;
#endif

  if (m_validate) {
    if (!AddExtensionName(extProps, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, m_instanceExtensionNames))
      return false;
  }

  App *app = App::Get();

  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = nullptr;
  appInfo.pApplicationName = app->m_name.c_str();
  appInfo.applicationVersion = app->m_ver;
  appInfo.pEngineName = "VGraphics";
  appInfo.engineVersion = 0;
  appInfo.apiVersion = VK_API_VERSION_1_0;

  std::vector<char const *> layerNames = GetPtrArray(m_instanceLayerNames), extNames = GetPtrArray(m_instanceExtensionNames);

  VkInstanceCreateInfo instInfo;
  instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instInfo.pNext = nullptr;
  instInfo.flags = 0;
  instInfo.pApplicationInfo = &appInfo;
  instInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
  instInfo.ppEnabledLayerNames = &layerNames.front();
  instInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
  instInfo.ppEnabledExtensionNames = &extNames.front();

  VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
  if (m_validate) {
    dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    dbgCreateInfo.pNext = nullptr;
    dbgCreateInfo.pfnCallback = DbgReportFunc;
    dbgCreateInfo.pUserData = this;
    dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    
    instInfo.pNext = &dbgCreateInfo;
  }

  err = vkCreateInstance(&instInfo, nullptr, &m_instance);
  if (err < 0) {
    std::cerr << "vkCreateInstance failed with error " << err << std::endl;
    return false;
  }

  return true;
}

void VGraphics::DoneInstance()
{
  if (m_instance) {
    vkDestroyInstance(m_instance, nullptr);
    m_instance = nullptr;
  }
}

#define GET_INSTANCE_PROC_ADDRESS(proc) \
  do { m_##proc = reinterpret_cast<PFN_vk##proc>(vkGetInstanceProcAddr(m_instance, "vk"#proc)); \
    if (!m_##proc) { \
      std::cerr << "GET_INSTANCE_PROC_ADDRESS() failed on " << "vk"#proc << std::endl; \
      return false; \
    } \
  } while (false)

#define GET_DEVICE_PROC_ADDRESS(proc) \
  do { m_##proc = reinterpret_cast<PFN_vk##proc>(vkGetDeviceProcAddr(m_device, "vk"#proc)); \
    if (!m_##proc) { \
      std::cerr << "GET_DEVICE_PROC_ADDRESS() failed on " << "vk"#proc << std::endl; \
      return false; \
    } \
  } while (false)


bool VGraphics::InitPhysicalDevice()
{
  uint32_t gpuCount;
  VkResult err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
  if (err < 0 || gpuCount == 0)
    return false;
  std::vector<VkPhysicalDevice> gpus(gpuCount);
  err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, &gpus.front());
  if (err < 0)
    return false;

  m_physicalDevice = gpus[0];

  std::vector<VkLayerProperties> layers;
  err = AppendData<VkLayerProperties>(layers, [this](uint32_t &s, VkLayerProperties *p)->VkResult { return vkEnumerateDeviceLayerProperties(m_physicalDevice, &s, p); });
  if (err < 0)
    return false;

  m_deviceLayerNames.clear();
  if (m_validate) {
    if (!AddLayerName(layers, "VK_LAYER_LUNARG_standard_validation", m_deviceLayerNames))
      return false;
  }

  std::vector<VkExtensionProperties> extensions;
  err = AppendData<VkExtensionProperties>(extensions, [this](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &s, p); });
  if (err < 0)
    return false;
  for (auto &l : m_deviceLayerNames) {
    err = AppendData<VkExtensionProperties>(extensions, [this, &l](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateDeviceExtensionProperties(m_physicalDevice, l.c_str(), &s, p); });
    if (err < 0)
      return false;
  }

  m_deviceExtensionNames.clear();
  if (!AddExtensionName(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME, m_deviceExtensionNames))
    return false;

  if (m_validate) {
    GET_INSTANCE_PROC_ADDRESS(CreateDebugReportCallbackEXT);
    GET_INSTANCE_PROC_ADDRESS(DestroyDebugReportCallbackEXT);
    GET_INSTANCE_PROC_ADDRESS(DebugReportMessageEXT);

    VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
    dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    dbgCreateInfo.pNext = nullptr;
    dbgCreateInfo.pfnCallback = DbgReportFunc;
    dbgCreateInfo.pUserData = this;
    dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    err = m_CreateDebugReportCallbackEXT(m_instance, &dbgCreateInfo, nullptr, &m_debugReportCallback);
    if (err < 0)
      return false;
  }

  vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProps);
  vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProps);

  err = AppendData<VkQueueFamilyProperties>(m_queueFamilies, [this](uint32_t &s, VkQueueFamilyProperties *p)->VkResult { vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &s, p); return VK_SUCCESS; });
  if (err < 0)
    return false;

  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfaceSupportKHR);
  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfaceFormatsKHR);
  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfacePresentModesKHR);
  GET_INSTANCE_PROC_ADDRESS(DestroySurfaceKHR);

  return true;
}

void VGraphics::DonePhysicalDevice()
{
  if (m_debugReportCallback) {
    m_DestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
    m_debugReportCallback = 0;
  }
}

bool VGraphics::InitSwapchain()
{
  VkResult err;
  App *app = App::Get();
  if (app->m_windows.empty())
    return false;
#if defined(_WIN32)
  VkWin32SurfaceCreateInfoKHR surfInfo;
  surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surfInfo.pNext = nullptr;
  surfInfo.flags = 0;
  surfInfo.hinstance = reinterpret_cast<HINSTANCE>(app->GetID());
  surfInfo.hwnd = reinterpret_cast<HWND>(app->m_windows[0]->GetID());
  err = vkCreateWin32SurfaceKHR(m_instance, &surfInfo, nullptr, &m_surface);
  if (err < 0)
    return false;
#elif defined(linux)
  VkXcbSurfaceCreateInfoKHR surfInfo;
  surfInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  surfInfo.pNext = nullptr;
  surfInfo.flags = 0;
  surfInfo.connection = reinterpret_cast<xcb_connection_t*>(app->GetID());
  surfInfo.window = reinterpret_cast<xcb_window_t>(app->m_windows[0]->GetID());
  err = vkCreateXcbSurfaceKHR(m_instance, &surfInfo, nullptr, &m_surface);
  if (err < 0)
    return false;
#endif

  std::vector<VkBool32> supportsPresent(m_queueFamilies.size());
  for (int i = 0; i < supportsPresent.size(); ++i) {
    err = m_GetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent[i]);
    if (err < 0)
      return false;
  }

  m_graphicsQueueFamily = -1;
  for (unsigned i = 0; i < m_queueFamilies.size(); ++i) {
    if ((m_queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent[i]) {
      m_graphicsQueueFamily = i;
      break;
    }
  }
  if (m_graphicsQueueFamily >= m_queueFamilies.size())
    return false;

  std::vector<VkSurfaceFormatKHR> surfFormats;
  err = AppendData<VkSurfaceFormatKHR>(surfFormats, [this](uint32_t &s, VkSurfaceFormatKHR *f)->VkResult { return m_GetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &s, f); });
  if (err < 0 || surfFormats.empty())
    return false;

  m_surfaceFormat = surfFormats[0];
  if (m_surfaceFormat.format == VK_FORMAT_UNDEFINED) // no preferred format
    m_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;

  return true;
}

void VGraphics::DoneSwapchain()
{
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

bool VGraphics::InitDevice()
{
  VkResult err;
  VkDeviceQueueCreateInfo queueInfo;
  float queuePriorities[] = {0.0};
  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfo.pNext = nullptr;
  queueInfo.queueFamilyIndex = m_graphicsQueueFamily;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = queuePriorities;

  std::vector<char const *> layerNames = GetPtrArray(m_deviceLayerNames), extNames = GetPtrArray(m_deviceExtensionNames);
  VkDeviceCreateInfo deviceInfo;
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.pNext = nullptr;
  deviceInfo.flags = 0;
  deviceInfo.queueCreateInfoCount = 1;
  deviceInfo.pQueueCreateInfos = &queueInfo;
  deviceInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
  deviceInfo.ppEnabledLayerNames = &layerNames.front();
  deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
  deviceInfo.ppEnabledExtensionNames = &extNames.front();
  deviceInfo.pEnabledFeatures = nullptr;

  err = vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device);
  if (err < 0)
    return false;

  VkQueue queue;
  vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &queue);
  m_queue = new VQueue(*this, true, queue);

  GET_DEVICE_PROC_ADDRESS(CreateSwapchainKHR);
  GET_DEVICE_PROC_ADDRESS(DestroySwapchainKHR);
  GET_DEVICE_PROC_ADDRESS(GetSwapchainImagesKHR);
  GET_DEVICE_PROC_ADDRESS(AcquireNextImageKHR);
  GET_DEVICE_PROC_ADDRESS(QueuePresentKHR);

  return true;
}

void VGraphics::DoneDevice()
{
  if (m_queue) {
    delete m_queue;
    m_queue = nullptr;
  }
  vkDestroyDevice(m_device, nullptr);
}

bool VGraphics::InitCmdBuffers(bool synchronize, uint32_t count)
{
  m_cmdPool = new VCmdPool(*this, synchronize, false, true);
  m_cmdPool->CreateBuffers(true, count, m_cmdBuffers);
  return true;
}

void VGraphics::DoneCmdBuffers()
{
  for (auto b : m_cmdBuffers)
    delete b;
  m_cmdBuffers.empty();
  if (m_cmdPool) {
    delete m_cmdPool;
    m_cmdPool = nullptr;
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VGraphics::DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
  return false;
}

VCmdPool::VCmdPool(VGraphics &graphics, bool synchronize, bool transient, bool resetBuffers): m_graphics(&graphics), m_lock(synchronize)
{
  VkCommandPoolCreateInfo poolInfo;
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.pNext = nullptr;
  poolInfo.flags = (transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0) | (resetBuffers ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0);
  poolInfo.queueFamilyIndex = m_graphics->m_graphicsQueueFamily;
  VkResult err = vkCreateCommandPool(m_graphics->m_device, &poolInfo, nullptr, &m_pool);
  assert(err >= 0);
}

VCmdPool::~VCmdPool()
{
  vkDestroyCommandPool(m_graphics->m_device, m_pool, nullptr);
}

void VCmdPool::InitBufferInfo(VkCommandBufferAllocateInfo &bufInfo, bool primary, uint32_t count)
{
  bufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  bufInfo.pNext = nullptr;
  bufInfo.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  bufInfo.commandPool = m_pool;
  bufInfo.commandBufferCount = count;
}

VCmdBuffer *VCmdPool::CreateBuffer(bool primary)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  VkCommandBufferAllocateInfo bufInfo;
  InitBufferInfo(bufInfo, primary, 1);
  VkCommandBuffer buffer;
  VkResult err = vkAllocateCommandBuffers(m_graphics->m_device, &bufInfo, &buffer);
  assert(err >= 0);
  return new VCmdBuffer(*this, buffer);
}

void VCmdPool::CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  VkCommandBufferAllocateInfo bufInfo;
  InitBufferInfo(bufInfo, primary, count);
  std::vector<VkCommandBuffer> vkBuffers(count);
  VkResult err = vkAllocateCommandBuffers(m_graphics->m_device, &bufInfo, &vkBuffers.front());
  assert(err >= 0);
  for (auto b : vkBuffers)
    buffers.push_back(new VCmdBuffer(*this, b));
}

VCmdBuffer::VCmdBuffer(VCmdPool &pool, VkCommandBuffer buffer) : m_pool(&pool), m_buffer(buffer) 
{
}

VCmdBuffer::~VCmdBuffer()
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  vkFreeCommandBuffers(m_pool->m_graphics->m_device, m_pool->m_pool, 1, &m_buffer);
}

VQueue::VQueue(VGraphics &graphics, bool synchronize, VkQueue queue): m_queue(queue), m_graphics(&graphics), m_lock(synchronize)
{
}

VQueue::~VQueue()
{
}
