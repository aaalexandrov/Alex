#include "stdafx.h"
#include "VGraphics.h"
#include "App.h"
#include "Wnd.h"

#include <iostream>
#include <functional>
#include <algorithm>

#pragma warning(push)
#pragma warning(disable: 4477)
#include "CImg.h"
#pragma warning(pop)

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

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
  if (!InitSurface())
    return false;
  if (!InitDevice())
    return false;

  return true;
}

void VGraphics::Done()
{
  DoneDevice();
  DoneSurface();
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

  m_layerNames.clear();
  if (m_validate) {
    if (!AddLayerName(layerProps, "VK_LAYER_LUNARG_standard_validation", m_layerNames))
      return false;
  }

  std::vector<VkExtensionProperties> extProps;
  err = AppendData<VkExtensionProperties>(extProps, [](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateInstanceExtensionProperties(nullptr, &s, p); });
  if (err < 0)
    return false;
  for (auto &l : m_layerNames) {
    err = AppendData<VkExtensionProperties>(extProps, [&l](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateInstanceExtensionProperties(l.c_str(), &s, p); });
    if (err < 0)
      return false;
  }
  
  m_extensionNames.clear();
  if (!AddExtensionName(extProps, VK_KHR_SURFACE_EXTENSION_NAME, m_extensionNames))
    return false;

#if defined(_WIN32)
  if (!AddExtensionName(extProps, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, m_extensionNames))
    return false;
#elif defined(linux)
  if (!AddExtensionName(extProps, VK_KHR_XCB_SURFACE_EXTENSION_NAME, extNames))
    return false;
#endif

  if (m_validate) {
    if (!AddExtensionName(extProps, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, m_extensionNames))
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

  std::vector<char const *> layerNames = GetPtrArray(m_layerNames), extNames = GetPtrArray(m_extensionNames);

  VkInstanceCreateInfo instInfo;
  instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instInfo.pNext = nullptr;
  instInfo.flags = 0;
  instInfo.pApplicationInfo = &appInfo;
  instInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
  instInfo.ppEnabledLayerNames = layerNames.data();
  instInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
  instInfo.ppEnabledExtensionNames = extNames.data();

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
  err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data());
  if (err < 0)
    return false;

  m_physicalDevice = gpus[0];

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

bool VGraphics::InitSurface()
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

void VGraphics::DoneSurface()
{
  vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

bool VGraphics::InitDevice()
{
  assert(!m_device);
  m_device = new VDevice(*this);

  if (!m_device->Init())
    return false;

  return true;
}

void VGraphics::DoneDevice()
{
  if (m_device) {
    m_device->Done();
    m_device = nullptr;
  }
}

uint32_t VGraphics::GetMemoryTypeIndex(uint32_t validTypeMask, VkMemoryPropertyFlags flags)
{
  for (uint32_t i = 0; i < m_physicalDeviceMemoryProps.memoryTypeCount; ++i)
    if ((validTypeMask & (1 << i)) && (m_physicalDeviceMemoryProps.memoryTypes[i].propertyFlags & flags) == flags)
      return i;
  return -1;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VGraphics::DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
  std::string flag = (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ? "error" : "warning";
  std::cerr << "Vulkan debug " << flag << " code: " << msgCode << " layer:" << pLayerPrefix << " msg: " << pMsg << std::endl;
  return false;
}

VCmdPool::VCmdPool(VDevice &device, bool synchronize, bool transient, bool resetBuffers): m_device(&device), m_lock(synchronize)
{
  VkCommandPoolCreateInfo poolInfo;
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.pNext = nullptr;
  poolInfo.flags = (transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0) | (resetBuffers ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0);
  poolInfo.queueFamilyIndex = m_device->m_graphics->m_graphicsQueueFamily;
  VkResult err = vkCreateCommandPool(m_device->m_device, &poolInfo, nullptr, &m_pool);
  assert(err >= 0);
}

VCmdPool::~VCmdPool()
{
  vkDestroyCommandPool(m_device->m_device, m_pool, nullptr);
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
  VkResult err = vkAllocateCommandBuffers(m_device->m_device, &bufInfo, &buffer);
  assert(err >= 0);
  return new VCmdBuffer(*this, buffer);
}

void VCmdPool::CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  VkCommandBufferAllocateInfo bufInfo;
  InitBufferInfo(bufInfo, primary, count);
  std::vector<VkCommandBuffer> vkBuffers(count);
  VkResult err = vkAllocateCommandBuffers(m_device->m_device, &bufInfo, vkBuffers.data());
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
  vkFreeCommandBuffers(m_pool->m_device->m_device, m_pool->m_pool, 1, &m_buffer);
}

void VCmdBuffer::SetUseSemaphore(bool use, VkPipelineStageFlags stages)
{
  if (use == !!m_semaphore)
    return;

  if (use)
    m_semaphore = new VSemaphore(*m_pool->m_device, stages);
  else {
    delete m_semaphore;
    m_semaphore = nullptr;
  }
}

void VCmdBuffer::SetUseFence(bool use)
{
  if (use == !!m_fence)
    return;

  if (use)
    m_fence = new VFence(*m_pool->m_device);
  else {
    delete m_fence;
    m_fence = nullptr;
  }
}

bool VCmdBuffer::AutoBegin()
{
  if (m_autoBegin && !m_begun)
    return Begin(m_simultaneousBegin);
  return true;
}

bool VCmdBuffer::AutoEnd()
{
  if (m_autoBegin && m_begun)
    return End();
  return true;
}

bool VCmdBuffer::Begin(bool simultaneous)
{
  VkCommandBufferInheritanceInfo inheritInfo;
  inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  inheritInfo.pNext = nullptr;
  inheritInfo.renderPass = VK_NULL_HANDLE;
  inheritInfo.subpass = 0;
  inheritInfo.framebuffer = VK_NULL_HANDLE;
  inheritInfo.occlusionQueryEnable = false;
  inheritInfo.queryFlags = 0;
  inheritInfo.pipelineStatistics = 0;

  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.flags = simultaneous ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0;
  beginInfo.pInheritanceInfo = &inheritInfo;

  std::lock_guard<ConditionalLock>(m_pool->m_lock);

  VkResult err = vkBeginCommandBuffer(m_buffer, &beginInfo);
  if (err < 0)
    return false;

  m_begun = true;

  return true;
}

bool VCmdBuffer::End()
{
  assert(m_begun);
  std::lock_guard<ConditionalLock>(m_pool->m_lock);
  VkResult err = vkEndCommandBuffer(m_buffer);
  if (err < 0)
    return false;

  m_begun = false;

  return true;
}

bool VCmdBuffer::Reset(bool releaseResources)
{
  std::lock_guard<ConditionalLock>(m_pool->m_lock);
  VkResult err = vkResetCommandBuffer(m_buffer, releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
  if (err < 0)
    return false;

  return true;
}

bool VCmdBuffer::SetImageLayout(VImage &image, VkImageLayout layout, VkAccessFlags priorAccess, VkAccessFlags followingAccess)
{
  if (!AutoBegin())
    return false;

  VkImageAspectFlags aspectFlags = VImage::AspectFromFormat(image.m_format);

  VkImageMemoryBarrier imgBarrier;
  imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imgBarrier.pNext = nullptr;
  imgBarrier.srcAccessMask = priorAccess;
  imgBarrier.dstAccessMask = followingAccess;
  imgBarrier.oldLayout = image.m_layout;
  imgBarrier.newLayout = layout;
  imgBarrier.image = image.m_image;
  imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imgBarrier.subresourceRange = { aspectFlags, 0, 1, 0, 1 };

  std::lock_guard<ConditionalLock>(m_pool->m_lock);

  vkCmdPipelineBarrier(m_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);

  image.m_layout = layout;

  return true;
}

bool VCmdBuffer::CopyImage(VImage &src, VImage &dst)
{
  if (!AutoBegin())
    return false;
  
  VkImageCopy copy;
  copy.srcSubresource = { VImage::AspectFromFormat(src.m_format), 0, 0, 1 };
  copy.srcOffset = { 0, 0, 0 };
  copy.dstSubresource = { VImage::AspectFromFormat(dst.m_format), 0, 0, 1 };
  copy.dstOffset = { 0, 0, 0 };
  copy.extent = { std::min(src.m_size.width, dst.m_size.width), std::min(src.m_size.height, dst.m_size.height), std::min(src.m_size.depth, dst.m_size.depth) };

  std::lock_guard<ConditionalLock>(m_pool->m_lock);

  vkCmdCopyImage(m_buffer, src.m_image, src.m_layout, dst.m_image, dst.m_layout, 1, &copy);

  return true;
}

bool VCmdBuffer::CopyBuffer(VBuffer &src, VBuffer &dst, uint64_t srcOffset, uint64_t dstOffset, uint64_t size)
{
  if (!AutoBegin())
    return false;

  VkBufferCopy copy;
  copy.size = std::min(size, std::min(src.m_size - srcOffset, dst.m_size - dstOffset));
  copy.srcOffset = srcOffset;
  copy.dstOffset = dstOffset;

  std::lock_guard<ConditionalLock>(m_pool->m_lock);

  vkCmdCopyBuffer(m_buffer, src.m_buffer, dst.m_buffer, 1, &copy);

  return true;
}

VQueue::VQueue(VDevice &device, bool synchronize, VkQueue queue): m_queue(queue), m_device(&device), m_lock(synchronize)
{
}

VQueue::~VQueue()
{
}

bool VQueue::Submit(VCmdBuffer &cmdBuffer, std::vector<VSemaphore*>* waitSemaphores)
{
  if (!cmdBuffer.AutoEnd())
    return false;

  std::vector<VkSemaphore> semaphores;
  std::vector<VkPipelineStageFlags> stages;
  if (waitSemaphores) {
    for (auto s : *waitSemaphores) {
      semaphores.push_back(s->m_semaphore);
      stages.push_back(s->m_stages);
    }
  }

  VkSubmitInfo subInfo;
  subInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  subInfo.pNext = nullptr;
  subInfo.commandBufferCount = 1;
  subInfo.pCommandBuffers = &cmdBuffer.m_buffer;
  subInfo.pWaitDstStageMask = stages.empty() ? nullptr : stages.data();
  subInfo.waitSemaphoreCount = (uint32_t) semaphores.size();
  subInfo.pWaitSemaphores = semaphores.empty() ? nullptr : semaphores.data();
  subInfo.signalSemaphoreCount = !!cmdBuffer.m_semaphore;
  subInfo.pSignalSemaphores = cmdBuffer.m_semaphore ? &cmdBuffer.m_semaphore->m_semaphore : nullptr;

  std::lock_guard<ConditionalLock> lock(m_lock);

  VkResult err = vkQueueSubmit(m_queue, 1, &subInfo, cmdBuffer.m_fence ? cmdBuffer.m_fence->m_fence : VK_NULL_HANDLE);
  if (err < 0)
    return false;

  return true;
}

VDevice::VDevice(VGraphics &graphics) : m_graphics(&graphics)
{
}

VDevice::~VDevice()
{
}

bool VDevice::Init()
{
  if (!InitCapabilities())
    return false;
  if (!InitDevice())
    return false;
  if (!InitCmdBuffers(true, 1))
    return false;

  if (!InitSwapchain())
    return false;

  if (!InitDepth())
    return false;

  VImage *img = LoadVImage("../../Terrain/Data/Textures/Earth.bmp");
  DoneResource(img);

  std::vector<char> trash(1024);
  VBuffer *buf = LoadVBuffer(trash.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, trash.data());
  DoneResource(buf);

  if (!SubmitInitCommands())
    return false;

  return true;
}

void VDevice::Done()
{
  DoneResource(m_staging);
  DoneDepth();
  DoneSwapchain();
  DoneCmdBuffers();
  DoneDevice();
  DoneCapabilities();
}

bool VDevice::InitCapabilities()
{
  std::vector<VkLayerProperties> layers;
  VkResult err = AppendData<VkLayerProperties>(layers, [this](uint32_t &s, VkLayerProperties *p)->VkResult { return vkEnumerateDeviceLayerProperties(m_graphics->m_physicalDevice, &s, p); });
  if (err < 0)
    return false;

  m_layerNames.clear();
  if (m_graphics->m_validate) {
    if (!VGraphics::AddLayerName(layers, "VK_LAYER_LUNARG_standard_validation", m_layerNames))
      return false;
  }

  std::vector<VkExtensionProperties> extensions;
  err = AppendData<VkExtensionProperties>(extensions, [this](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateDeviceExtensionProperties(m_graphics->m_physicalDevice, nullptr, &s, p); });
  if (err < 0)
    return false;
  for (auto &l : m_layerNames) {
    err = AppendData<VkExtensionProperties>(extensions, [this, &l](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateDeviceExtensionProperties(m_graphics->m_physicalDevice, l.c_str(), &s, p); });
    if (err < 0)
      return false;
  }

  m_extensionNames.clear();
  if (!VGraphics::AddExtensionName(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME, m_extensionNames))
    return false;

  return true;
}

void VDevice::DoneCapabilities()
{
}

bool VDevice::InitDevice()
{
  VkResult err;
  VkDeviceQueueCreateInfo queueInfo;
  float queuePriorities[] = { 0.0 };
  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfo.pNext = nullptr;
  queueInfo.queueFamilyIndex = m_graphics->m_graphicsQueueFamily;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = queuePriorities;

  std::vector<char const *> layerNames = VGraphics::GetPtrArray(m_layerNames), extNames = VGraphics::GetPtrArray(m_extensionNames);
  VkDeviceCreateInfo deviceInfo;
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.pNext = nullptr;
  deviceInfo.flags = 0;
  deviceInfo.queueCreateInfoCount = 1;
  deviceInfo.pQueueCreateInfos = &queueInfo;
  deviceInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
  deviceInfo.ppEnabledLayerNames = layerNames.data();
  deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
  deviceInfo.ppEnabledExtensionNames = extNames.data();
  deviceInfo.pEnabledFeatures = nullptr;

  err = vkCreateDevice(m_graphics->m_physicalDevice, &deviceInfo, nullptr, &m_device);
  if (err < 0)
    return false;

  VkQueue queue;
  vkGetDeviceQueue(m_device, m_graphics->m_graphicsQueueFamily, 0, &queue);
  m_queue = new VQueue(*this, true, queue);

  GET_DEVICE_PROC_ADDRESS(CreateSwapchainKHR);
  GET_DEVICE_PROC_ADDRESS(DestroySwapchainKHR);
  GET_DEVICE_PROC_ADDRESS(GetSwapchainImagesKHR);
  GET_DEVICE_PROC_ADDRESS(AcquireNextImageKHR);
  GET_DEVICE_PROC_ADDRESS(QueuePresentKHR);

  return true;
}

void VDevice::DoneDevice()
{
  if (m_queue) {
    delete m_queue;
    m_queue = nullptr;
  }
  vkDestroyDevice(m_device, nullptr);
}

bool VDevice::InitSwapchain()
{
  assert(!m_swapchain);
  m_swapchain = new VSwapchain(*this);
  if (!m_swapchain->Init())
    return false;

  return true;
}

void VDevice::DoneSwapchain()
{
  if (m_swapchain) {
    m_swapchain->Done();
    delete m_swapchain;
    m_swapchain = nullptr;
  }
}

bool VDevice::InitDepth()
{
  VkSurfaceCapabilitiesKHR surfaceCaps;
  VkResult err = m_graphics->m_GetPhysicalDeviceSurfaceCapabilitiesKHR(m_graphics->m_physicalDevice, m_graphics->m_surface, &surfaceCaps);
  if (err < 0)
    return false;

  m_depth = new VImage(*this);
  if (!m_depth->Init(VK_FORMAT_D24_UNORM_S8_UINT, surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height))
    return false;

  if (!m_cmdBuffer->SetImageLayout(*m_depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT))
    return false;

  return true;
}

void VDevice::DoneDepth()
{
  if (m_depth) {
    m_depth->Done();
    delete m_depth;
    m_depth = nullptr;
  }
}

bool VDevice::InitCmdBuffers(bool synchronize, uint32_t count)
{
  m_cmdPool = new VCmdPool(*this, synchronize, false, true);
  m_cmdBuffer = m_cmdPool->CreateBuffer(true);

  return true;
}

void VDevice::DoneCmdBuffers()
{
  if (m_cmdBuffer) {
    delete m_cmdBuffer;
    m_cmdBuffer = nullptr;
  }
  if (m_cmdPool) {
    delete m_cmdPool;
    m_cmdPool = nullptr;
  }
}

bool VDevice::SubmitInitCommands()
{
  if (!m_queue->Submit(*m_cmdBuffer))
    return false;

  VkResult err = vkQueueWaitIdle(m_queue->m_queue);
  if (err < 0)
    return false;

  if (!m_cmdBuffer->Reset())
    return false;

  if (!m_cmdBuffer->Begin())
    return false;

  return true;
}

VImage *VDevice::LoadVImage(std::string const &filename)
{
  VImage *vimg = nullptr;
  try {
    cimg_library::cimg::exception_mode(1);
    cimg_library::CImg<unsigned char> image(filename.c_str());
    uint32_t width = image.width(), height = image.height(), depth = image.depth(), components = image.spectrum();

    if (!UpdateStagingImage(width, height, depth, components))
      return nullptr;

    cimg_library::CImg<unsigned char> imgRGBA;
    
    if (components < 4) {
      imgRGBA = image.get_channels(0, 3);
      if (components < 2)
        imgRGBA.get_shared_channels(components + 1, 2) = static_cast<unsigned char>(0);
      imgRGBA.get_shared_channel(3) = 255;
    } else
      imgRGBA = image.get_shared_channels(0, 3);

    imgRGBA.permute_axes("cxyz");

    void *mem = m_staging->Map();
    memcpy(mem, imgRGBA.data(), imgRGBA.size());
    m_staging->Unmap();

    vimg = new VImage(*this);
    VkFormat format = VImage::FormatFromComponents(components);
    if (!vimg->Init(format, width, height, depth)) {
      delete vimg;
      return nullptr;
    }

  } catch (...) {
    return nullptr;
  }

  AutoDone<VImage> doneVimg(vimg);

  if (m_staging->m_layout != VK_IMAGE_LAYOUT_GENERAL && !m_cmdBuffer->SetImageLayout(*m_staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT))
    return false;

  if (!m_cmdBuffer->SetImageLayout(*vimg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT))
    return false;

  if (!m_cmdBuffer->CopyImage(*m_staging, *vimg))
    return false;

  if (!m_cmdBuffer->SetImageLayout(*vimg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT))
    return false;

  if (!m_cmdBuffer->SetImageLayout(*m_staging, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_HOST_WRITE_BIT))
    return false;

  if (!SubmitInitCommands())
    return nullptr;

  doneVimg.m_ptr = nullptr;
  return vimg;
}

VBuffer *VDevice::LoadVBuffer(uint64_t size, VkBufferUsageFlags usage, void *data)
{
  VBuffer staging(*this, false), *buffer = nullptr;

  if (!staging.Init(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true))
    return nullptr;

  AutoDone<VBuffer> doneStaging(&staging, false);

  void *stagingMem = staging.Map();
  memcpy(stagingMem, data, size);
  staging.Unmap();

  buffer = new VBuffer(*this, false);
  AutoDone<VBuffer> bufferDone(buffer);

  if (!buffer->Init(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false))
    return nullptr;

  if (!m_cmdBuffer->CopyBuffer(staging, *buffer))
    return nullptr;

  if (!SubmitInitCommands())
    return nullptr;

  bufferDone.m_ptr = nullptr;
  return buffer;
}

bool VDevice::UpdateStagingImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t components)
{
  if (m_staging)
    if (m_staging->m_size.width < width || m_staging->m_size.height < height || m_staging->m_size.depth < depth)
      DoneResource(m_staging);
    else
      return true;
  m_staging = new VImage(*this);
  if (!m_staging->Init(VK_FORMAT_R8G8B8A8_UNORM, width, height, depth, 1, 1, true))
    return false;

  return true;
}

void VDevice::FreeStagingImage()
{
  DoneResource(m_staging);
}

VSwapchain::VSwapchain(VDevice &device) : m_device(&device)
{
}

VSwapchain::~VSwapchain()
{
}

bool VSwapchain::Init()
{
  if (!InitSwapchain())
    return false;
  if (!InitImages())
    return false;

  return true;
}

void VSwapchain::Done()
{
  DoneImages();
  DoneSwapchain();
}

bool VSwapchain::InitSwapchain()
{
  VkResult err;
  VkSurfaceCapabilitiesKHR surfaceCaps;
  err = m_device->m_graphics->m_GetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_graphics->m_physicalDevice, m_device->m_graphics->m_surface, &surfaceCaps);
  if (err < 0)
    return false;

  std::vector<VkPresentModeKHR> presentModes;
  err = AppendData<VkPresentModeKHR>(presentModes, [this](uint32_t &s, VkPresentModeKHR *m)->VkResult { return m_device->m_graphics->m_GetPhysicalDeviceSurfacePresentModesKHR(m_device->m_graphics->m_physicalDevice, m_device->m_graphics->m_surface, &s, m); });
  if (err < 0)
    return false;

  VkPresentModeKHR mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
  for (auto m : { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR }) {
    if (std::find(presentModes.begin(), presentModes.end(), m) != presentModes.end()) {
      mode = m;
      break;
    }
  }
  if (mode == VK_PRESENT_MODE_MAX_ENUM_KHR)
    return false;

  VkSwapchainKHR oldChain = m_swapchain;

  VkSwapchainCreateInfoKHR chainInfo;
  chainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  chainInfo.pNext = nullptr;
  chainInfo.flags = 0;
  chainInfo.surface = m_device->m_graphics->m_surface;
  chainInfo.minImageCount = surfaceCaps.minImageCount + 1;
  chainInfo.imageFormat = m_device->m_graphics->m_surfaceFormat.format;
  chainInfo.imageColorSpace = m_device->m_graphics->m_surfaceFormat.colorSpace;
  chainInfo.imageExtent = surfaceCaps.currentExtent;
  chainInfo.imageArrayLayers = 1;
  chainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  chainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  chainInfo.queueFamilyIndexCount = 0;
  chainInfo.pQueueFamilyIndices = nullptr;
  chainInfo.preTransform = (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfaceCaps.currentTransform;
  chainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  chainInfo.presentMode = mode;
  chainInfo.clipped = true;
  chainInfo.oldSwapchain = oldChain;

  err = m_device->m_CreateSwapchainKHR(m_device->m_device, &chainInfo, nullptr, &m_swapchain);
  if (err < 0)
    return false;

  if (oldChain) {
    DoneImages(); // Images are deleted when their swapchain is destroyed, so we get rid of them too
    m_device->m_DestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);
  }

  return true;
}

void VSwapchain::DoneSwapchain()
{
  m_device->m_DestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);
}

bool VSwapchain::InitImages()
{
  std::vector<VkImage> images;
  VkResult err = AppendData<VkImage>(images, [this](uint32_t &s, VkImage *i)->VkResult { return m_device->m_GetSwapchainImagesKHR(m_device->m_device, m_swapchain, &s, i); });
  if (err < 0)
    return false;

  for (auto im : images) {
    VImage *img = new VImage(*m_device);
    m_images.push_back(img);
    if (!img->Init(im, m_device->m_graphics->m_surfaceFormat.format, false))
      return false;
    if (!m_device->m_cmdBuffer->SetImageLayout(*img, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, 0))
      return false;
  }

  return true;
}

void VSwapchain::DoneImages()
{
  for (auto image : m_images) {
    image->Done();
    delete image;
  }
  m_images.clear();
}

VImage::VImage(VDevice &device, bool synchronize) : m_device(&device), m_memory(synchronize)
{
}

VImage::~VImage()
{
}

VkImageUsageFlags VImage::UsageFromFormat(VkFormat format)
{
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    default:
      return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  }
}

VkImageAspectFlags VImage::AspectFromFormat(VkFormat format)
{
  switch (format) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
      return VK_IMAGE_ASPECT_DEPTH_BIT;
    case VK_FORMAT_S8_UINT:
      return VK_IMAGE_ASPECT_STENCIL_BIT;
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default:
      return VK_IMAGE_ASPECT_COLOR_BIT;
  }
}

VkFormat VImage::FormatFromComponents(unsigned components)
{
  switch (components) {
    case 1:
      return VK_FORMAT_R8_UNORM;
    case 2:
      return VK_FORMAT_R8G8_UNORM;
    case 3:
    case 4:
      return VK_FORMAT_R8G8B8A8_UNORM;
  }
  return VkFormat();
}

VkImageViewType VImage::ViewTypeFromDimensions(uint32_t width, uint32_t height, uint32_t depth, uint32_t arrayLayers)
{
  if (depth == 1)
    if (arrayLayers == 1)
      return VK_IMAGE_VIEW_TYPE_2D;
    else
      return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  else
    return VK_IMAGE_VIEW_TYPE_3D;
}

bool VImage::Init(VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, bool linear)
{
  m_format = format;
  m_layout = linear ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED;
  m_size = { width, height, depth };

  VkImageCreateInfo imgInfo;
  imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imgInfo.pNext = nullptr;
  imgInfo.flags = 0;
  imgInfo.imageType = depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
  imgInfo.extent = m_size;
  imgInfo.format = format;
  imgInfo.mipLevels = mipLevels;
  imgInfo.arrayLayers = arrayLayers;
  imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imgInfo.usage = linear ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT :  UsageFromFormat(format);
  imgInfo.tiling = linear ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
  imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imgInfo.initialLayout = m_layout;
  imgInfo.queueFamilyIndexCount = 0;
  imgInfo.pQueueFamilyIndices = nullptr;

  VkImage image;
  VkResult err = vkCreateImage(m_device->m_device, &imgInfo, nullptr, &image);
  if (err < 0)
    return false;

  m_image = image;
  m_ownImage = true;

  VkMemoryRequirements imgMem;
  vkGetImageMemoryRequirements(m_device->m_device, image, &imgMem);

  if (!m_memory.Init(*m_device, imgMem.size, imgMem.memoryTypeBits, linear ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : 0))
    return false;

  err = vkBindImageMemory(m_device->m_device, image, m_memory.m_memory, 0);
  if (err < 0)
    return false;

  if (!linear && !Init(image, format, true, ViewTypeFromDimensions(width, height, depth, arrayLayers)))
    return false;

  return true;
}

bool VImage::Init(VkImage image, VkFormat format, bool ownImage, VkImageViewType imgType)
{
  m_format = format;
  m_image = image;
  m_ownImage = ownImage;

  VkImageViewCreateInfo viewInfo;
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = nullptr;
  viewInfo.flags = 0;
  viewInfo.image = m_image;
  viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
  viewInfo.format = format;
  viewInfo.viewType = imgType;
  viewInfo.subresourceRange = { AspectFromFormat(format), 0, 1, 0, 1 };

  VkResult err = vkCreateImageView(m_device->m_device, &viewInfo, nullptr, &m_view);
  if (err < 0)
    return false;

  return true;
}

void VImage::Done()
{
  if (m_view) {
    vkDestroyImageView(m_device->m_device, m_view, nullptr);
    m_view = nullptr;
  }
  if (m_ownImage) {
    m_memory.Done(*m_device);
    vkDestroyImage(m_device->m_device, m_image, nullptr);
  }
  m_image = nullptr;
}

void *VImage::Map()
{
  return m_memory.Map(*m_device);
}

void VImage::Unmap()
{
  m_memory.Unmap(*m_device);
}

VSemaphore::VSemaphore(VDevice &device, VkPipelineStageFlags stages) : m_device(&device), m_stages(stages)
{
  VkSemaphoreCreateInfo semInfo;
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semInfo.pNext = nullptr;
  semInfo.flags = 0;

  VkResult err = vkCreateSemaphore(m_device->m_device, &semInfo, nullptr, &m_semaphore);
  assert(err >= 0);
}

VSemaphore::~VSemaphore()
{
  vkDestroySemaphore(m_device->m_device, m_semaphore, nullptr);
}

VFence::VFence(VDevice &device, bool signaled) : m_device(&device)
{
  VkFenceCreateInfo fenceInfo;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = nullptr;
  fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

  VkResult err = vkCreateFence(m_device->m_device, &fenceInfo, nullptr, &m_fence);
  assert(err >= 0);
}

VFence::~VFence()
{
  vkDestroyFence(m_device->m_device, m_fence, nullptr);
}

bool VFence::IsSignaled()
{
  VkResult err = vkGetFenceStatus(m_device->m_device, m_fence);
  return err == VK_SUCCESS;
}

void VFence::Reset()
{
  vkResetFences(m_device->m_device, 1, &m_fence);
}

bool VFence::Wait(uint64_t nsTimeout)
{
  VkResult err = vkWaitForFences(m_device->m_device, 1, &m_fence, true, nsTimeout);
  return err == VK_SUCCESS;
}

VSampler::VSampler(VDevice & device): m_device(&device)
{
}

VSampler::~VSampler()
{
}

bool VSampler::Init(VkSamplerAddressMode addressMode, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, float maxAnisotropy)
{
  VkSamplerCreateInfo samplerInfo;
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.pNext = nullptr;
  samplerInfo.flags = 0;
  samplerInfo.addressModeU = addressMode;
  samplerInfo.addressModeV = addressMode;
  samplerInfo.addressModeW = addressMode;
  samplerInfo.magFilter = magFilter;
  samplerInfo.minFilter = minFilter;
  samplerInfo.mipmapMode = mipmapMode;
  samplerInfo.anisotropyEnable = maxAnisotropy > 0.0f;
  samplerInfo.maxAnisotropy = maxAnisotropy;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.compareEnable = false;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  samplerInfo.unnormalizedCoordinates = false;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  VkResult err = vkCreateSampler(m_device->m_device, &samplerInfo, nullptr, &m_sampler);
  if (err < 0)
    return false;

  return true;
}

void VSampler::Done()
{
  vkDestroySampler(m_device->m_device, m_sampler, nullptr);
}

VMemory::VMemory(bool synchronize) : m_lock(synchronize)
{
}

VMemory::~VMemory()
{
  assert(!m_memory);
}

bool VMemory::Init(VDevice &device, uint64_t size, uint32_t validMemoryTypes, VkMemoryPropertyFlags memFlags)
{
  VkMemoryAllocateInfo memInfo;
  memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memInfo.pNext = nullptr;
  memInfo.allocationSize = size;
  memInfo.memoryTypeIndex = device.m_graphics->GetMemoryTypeIndex(validMemoryTypes, memFlags);
  VkResult err = vkAllocateMemory(device.m_device, &memInfo, nullptr, &m_memory);
  if (err < 0)
    return nullptr;
  m_size = size;
  return true;
}

void VMemory::Done(VDevice &device)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  vkFreeMemory(device.m_device, m_memory, nullptr);
  m_memory = VK_NULL_HANDLE;
}

void *VMemory::Map(VDevice &device, uint64_t offset, uint64_t size)
{
  void *mapped;
  std::lock_guard<ConditionalLock> lock(m_lock);
  VkResult err = vkMapMemory(device.m_device, m_memory, offset, size, 0, &mapped);
  if (err < 0)
    return nullptr;
  return mapped;
}

void VMemory::Unmap(VDevice &device)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  vkUnmapMemory(device.m_device, m_memory);
}

VBuffer::VBuffer(VDevice &device, bool synchronize) : m_device(&device), m_memory(synchronize)
{
}

VBuffer::~VBuffer()
{
}

bool VBuffer::Init(uint64_t size, VkBufferUsageFlags usage, bool hostVisible)
{
  m_usage = usage;
  m_size = size;

  VkBufferCreateInfo bufInfo;
  bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufInfo.pNext = nullptr;
  bufInfo.flags = 0;
  bufInfo.size = m_size;
  bufInfo.usage = m_usage;
  bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  bufInfo.queueFamilyIndexCount = 0;
  bufInfo.pQueueFamilyIndices = nullptr;

  VkResult err = vkCreateBuffer(m_device->m_device, &bufInfo, nullptr, &m_buffer);
  if (err < 0)
    return false;

  VkMemoryRequirements memReqs;
  vkGetBufferMemoryRequirements(m_device->m_device, m_buffer, &memReqs);

  if (!m_memory.Init(*m_device, memReqs.size, memReqs.memoryTypeBits, hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : 0))
    return false;

  err = vkBindBufferMemory(m_device->m_device, m_buffer, m_memory.m_memory, 0);
  if (err < 0)
    return false;

  // The following usages require a buffer view, we don't care for them now
  assert(!(m_usage & (VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)));

  return true;
}

void VBuffer::Done()
{
  m_memory.Done(*m_device);
  vkDestroyBuffer(m_device->m_device, m_buffer, nullptr);
}

void *VBuffer::Map()
{
  return m_memory.Map(*m_device);
}

void VBuffer::Unmap()
{
  m_memory.Unmap(*m_device);
}
