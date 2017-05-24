#include "stdafx.h"
#include "VGraphics.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <array>

#pragma warning(push)
#pragma warning(disable: 4477)
#include "../cimg/CImg.h"
#pragma warning(pop)

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

VGraphics::VGraphics(bool validate, std::string const &appName, uint32_t appVersion, uintptr_t instanceID, uintptr_t windowID) :
  m_validate(validate),
  m_instance(VK_NULL_HANDLE, [](VkInstance &instance) { vkDestroyInstance(instance, nullptr); }, false),
  m_surface(VK_NULL_HANDLE, [this](VkSurfaceKHR &surface) { vkDestroySurfaceKHR(m_instance, surface, nullptr); }, false),
  m_debugReportCallback(VK_NULL_HANDLE, [this](VkDebugReportCallbackEXT &callback) { if (callback) m_DestroyDebugReportCallbackEXT(m_instance, callback, nullptr); }, false)
{
  InitInstance(appName, appVersion);
  InitPhysicalDevice();
  InitSurface(instanceID, windowID);
  m_device.reset(new VDevice(*this));
}

struct VPipelineCacheDataHeader {
  uint32_t headerSize;
  VkPipelineCacheHeaderVersion headerVersion;
  uint32_t vendorID;
  uint32_t deviceID;
  uint8_t  pipelineCacheUUID[VK_UUID_SIZE];
};

bool VGraphics::IsPipelineCacheDataCompatible(std::vector<uint8_t> const &data)
{
  if (data.size() < sizeof(VPipelineCacheDataHeader))
    return false;
  VPipelineCacheDataHeader const *header = (VPipelineCacheDataHeader const *)data.data();
  if (header->headerSize != sizeof(VPipelineCacheDataHeader) || 
    header->headerVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE ||
    header->vendorID != m_physicalDeviceProps.vendorID || 
    header->deviceID != m_physicalDeviceProps.deviceID ||
    memcmp(header->pipelineCacheUUID, m_physicalDeviceProps.pipelineCacheUUID, VK_UUID_SIZE) != 0)
    return false;
  return true;
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

void VGraphics::InitInstance(std::string const &appName, uint32_t appVersion)
{
  VkResult err;
  std::vector<VkLayerProperties> layerProps;
  err = AppendData<VkLayerProperties>(layerProps, [](uint32_t &s, VkLayerProperties *p)->VkResult { return vkEnumerateInstanceLayerProperties(&s, p); });
  if (err < 0)
    throw VGraphicsException("VGraphics::InitInstance failed in vkEnumerateInstanceLayerProperties", err);

  m_layerNames.clear();
  if (m_validate) {
    if (!AddLayerName(layerProps, "VK_LAYER_LUNARG_standard_validation", m_layerNames))
      throw VGraphicsException("VGraphics::InitInstance failed to add validation layer", VK_RESULT_MAX_ENUM);
  }

  std::vector<VkExtensionProperties> extProps;
  err = AppendData<VkExtensionProperties>(extProps, [](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateInstanceExtensionProperties(nullptr, &s, p); });
  if (err < 0)
    throw VGraphicsException("VGraphics::InitInstance failed in vkEnumerateInstanceExtensionProperties", err);
  for (auto &l : m_layerNames) {
    err = AppendData<VkExtensionProperties>(extProps, [&l](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateInstanceExtensionProperties(l.c_str(), &s, p); });
    if (err < 0)
      throw VGraphicsException("VGraphics::InitInstance failed in vkEnumerateInstanceExtensionProperties", err);
  }
  
  m_extensionNames.clear();
  if (!AddExtensionName(extProps, VK_KHR_SURFACE_EXTENSION_NAME, m_extensionNames))
    throw VGraphicsException("VGraphics::InitInstance failed to add extension VK_KHR_SURFACE_EXTENSION_NAME", VK_RESULT_MAX_ENUM);

#if defined(_WIN32)
  if (!AddExtensionName(extProps, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, m_extensionNames))
    throw VGraphicsException("VGraphics::InitInstance failed to add extension VK_KHR_WIN32_SURFACE_EXTENSION_NAME", VK_RESULT_MAX_ENUM);
#elif defined(linux)
  if (!AddExtensionName(extProps, VK_KHR_XCB_SURFACE_EXTENSION_NAME, extNames))
    throw VGraphicsException("VGraphics::InitInstance failed to add extension VK_KHR_XCB_SURFACE_EXTENSION_NAME", VK_RESULT_MAX_ENUM);
#endif

  if (m_validate) {
    if (!AddExtensionName(extProps, VK_EXT_DEBUG_REPORT_EXTENSION_NAME, m_extensionNames))
      throw VGraphicsException("VGraphics::InitInstance failed to add extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME", VK_RESULT_MAX_ENUM);
  }

  VkApplicationInfo appInfo;
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pNext = nullptr;
  appInfo.pApplicationName = appName.c_str();
  appInfo.applicationVersion = appVersion;
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

  VkInstance instance = VK_NULL_HANDLE;
  err = vkCreateInstance(&instInfo, nullptr, &instance);
  if (err < 0) {
    std::cerr << "vkCreateInstance failed with error " << err << std::endl;
    throw VGraphicsException("VGraphics::InitInstance failed in vkCreateInstance", err);
  }
  m_instance.Reset(std::move(instance));
}

#define GET_INSTANCE_PROC_ADDRESS(proc) \
  do { m_##proc = reinterpret_cast<PFN_vk##proc>(vkGetInstanceProcAddr(m_instance, "vk"#proc)); \
    if (!m_##proc) { \
      std::cerr << "GET_INSTANCE_PROC_ADDRESS() failed on " << "vk"#proc << std::endl; \
      throw VGraphicsException("GET_INSTANCE_PROC_ADDRESS failed in vkGetInstanceProcAddr", VK_RESULT_MAX_ENUM); \
    } \
  } while (false)

#define GET_DEVICE_PROC_ADDRESS(proc) \
  do { m_##proc = reinterpret_cast<PFN_vk##proc>(vkGetDeviceProcAddr(m_device, "vk"#proc)); \
    if (!m_##proc) { \
      std::cerr << "GET_DEVICE_PROC_ADDRESS() failed on " << "vk"#proc << std::endl; \
      throw VGraphicsException("GET_DEVICE_PROC_ADDRESS failed in vkGetDeviceProcAddr", VK_RESULT_MAX_ENUM); \
    } \
  } while (false)


void VGraphics::InitPhysicalDevice()
{
  uint32_t gpuCount;
  VkResult err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
  if (err < 0 || gpuCount == 0)
    throw VGraphicsException("VGraphics::InitPhysicalDevice failed in vkEnumeratePhysicalDevices", err);
  std::vector<VkPhysicalDevice> gpus(gpuCount);
  err = vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data());
  if (err < 0)
    throw VGraphicsException("VGraphics::InitPhysicalDevice failed in vkEnumeratePhysicalDevices", err);

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
    VkDebugReportCallbackEXT callback = VK_NULL_HANDLE;
    err = m_CreateDebugReportCallbackEXT(m_instance, &dbgCreateInfo, nullptr, &callback);
    if (err < 0)
      throw VGraphicsException("VGraphics::InitPhysicalDevice failed in CreateDebugReportCallbackEXT", err);
    m_debugReportCallback.Reset(std::move(callback));
  }

  vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProps);
  vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProps);

  err = AppendData<VkQueueFamilyProperties>(m_queueFamilies, [this](uint32_t &s, VkQueueFamilyProperties *p)->VkResult { vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &s, p); return VK_SUCCESS; });
  if (err < 0)
    throw VGraphicsException("VGraphics::InitPhysicalDevice failed in vkGetPhysicalDeviceQueueFamilyProperties", err);

  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfaceSupportKHR);
  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfaceFormatsKHR);
  GET_INSTANCE_PROC_ADDRESS(GetPhysicalDeviceSurfacePresentModesKHR);
  GET_INSTANCE_PROC_ADDRESS(DestroySurfaceKHR);
}

void VGraphics::InitSurface(uintptr_t instanceID, uintptr_t windowID)
{
  VkResult err;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
#if defined(_WIN32)
  VkWin32SurfaceCreateInfoKHR surfInfo;
  surfInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surfInfo.pNext = nullptr;
  surfInfo.flags = 0;
  surfInfo.hinstance = reinterpret_cast<HINSTANCE>(instanceID);
  surfInfo.hwnd = reinterpret_cast<HWND>(windowID);
  err = vkCreateWin32SurfaceKHR(m_instance, &surfInfo, nullptr, &surface);
#elif defined(linux)
  VkXcbSurfaceCreateInfoKHR surfInfo;
  surfInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  surfInfo.pNext = nullptr;
  surfInfo.flags = 0;
  surfInfo.connection = reinterpret_cast<xcb_connection_t*>(instanceID);
  surfInfo.window = reinterpret_cast<xcb_window_t>(windowID);
  err = vkCreateXcbSurfaceKHR(m_instance, &surfInfo, nullptr, &surface);
#endif
  if (err < 0)
    throw VGraphicsException("VGraphics::InitSurface failed in vkCreateXXXXSurfaceKHR", err);
  m_surface.Reset(std::move(surface));

  std::vector<VkBool32> supportsPresent(m_queueFamilies.size());
  for (int i = 0; i < supportsPresent.size(); ++i) {
    err = m_GetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent[i]);
    if (err < 0)
      throw VGraphicsException("VGraphics::InitSurface failed in m_GetPhysicalDeviceSurfaceSupportKHR", err);
  }

  m_graphicsQueueFamily = -1;
  for (unsigned i = 0; i < m_queueFamilies.size(); ++i) {
    if ((m_queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent[i]) {
      m_graphicsQueueFamily = i;
      break;
    }
  }
  if (m_graphicsQueueFamily >= m_queueFamilies.size())
    throw VGraphicsException("VGraphics::InitSurface failed to find a graphics queue family", err);

  std::vector<VkSurfaceFormatKHR> surfFormats;
  err = AppendData<VkSurfaceFormatKHR>(surfFormats, [this](uint32_t &s, VkSurfaceFormatKHR *f)->VkResult { return m_GetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &s, f); });
  if (err < 0 || surfFormats.empty())
    throw VGraphicsException("VGraphics::InitSurface failed in GetPhysicalDeviceSurfaceFormatsKHR", err);

  m_surfaceFormat = surfFormats[0];
  if (m_surfaceFormat.format == VK_FORMAT_UNDEFINED) // no preferred format
    m_surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
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
  if (err < 0)
    throw VGraphicsException("VCmdPool failed in vkCreateCommandPool", err);
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
  if (err < 0)
    throw VGraphicsException("VCmdPool::CreateBuffer failed in vkAllocateCommandBuffers", err);
  return new VCmdBuffer(*this, buffer, primary);
}

void VCmdPool::CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  VkCommandBufferAllocateInfo bufInfo;
  InitBufferInfo(bufInfo, primary, count);
  std::vector<VkCommandBuffer> vkBuffers(count);
  VkResult err = vkAllocateCommandBuffers(m_device->m_device, &bufInfo, vkBuffers.data());
  if (err < 0)
    throw VGraphicsException("VCmdPool::CreateBuffers failed in vkAllocateCommandBuffers", err);
  for (auto b : vkBuffers)
    buffers.push_back(new VCmdBuffer(*this, b, primary));
}

VCmdBuffer::VCmdBuffer(VCmdPool &pool, VkCommandBuffer buffer, bool primary) : m_pool(&pool), m_buffer(buffer), m_primary(primary), m_autoBegin(primary)
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

void VCmdBuffer::AutoBegin()
{
  if (m_autoBegin && !m_begun)
    Begin(m_simultaneousBegin);
}

void VCmdBuffer::AutoEnd()
{
  if (m_autoBegin && m_begun)
    End();
}

void VCmdBuffer::Begin(bool simultaneous, VRenderPass *renderPass, uint32_t subpass)
{
  VkCommandBufferInheritanceInfo inheritInfo;
  inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
  inheritInfo.pNext = nullptr;
  inheritInfo.renderPass = renderPass ? renderPass->m_renderPass : VK_NULL_HANDLE;
  inheritInfo.subpass = subpass;
  inheritInfo.framebuffer = VK_NULL_HANDLE;
  inheritInfo.occlusionQueryEnable = false;
  inheritInfo.queryFlags = 0;
  inheritInfo.pipelineStatistics = 0;

  VkCommandBufferBeginInfo beginInfo;
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.flags = (simultaneous ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0) | (renderPass ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0);
  beginInfo.pInheritanceInfo = &inheritInfo;

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);

  VkResult err = vkBeginCommandBuffer(m_buffer, &beginInfo);
  if (err < 0)
    throw VGraphicsException("VCmdBuffer::Begin failed in vkBeginCommandBuffer", err);

  m_begun = true;
}

void VCmdBuffer::End()
{
  assert(m_begun);
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  VkResult err = vkEndCommandBuffer(m_buffer);
  if (err < 0)
    throw VGraphicsException("VCmdBuffer::End failed in vkEndCommandBuffer", err);

  m_begun = false;
}

void VCmdBuffer::Reset(bool releaseResources)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  VkResult err = vkResetCommandBuffer(m_buffer, releaseResources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
  if (err < 0)
    throw VGraphicsException("VCmdBuffer::Reset failed in vkResetCommandBuffer", err);
}

void VCmdBuffer::SetImageLayout(VImage &image, VkImageLayout layout, VkAccessFlags priorAccess, VkAccessFlags followingAccess)
{
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

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdPipelineBarrier(m_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);

  image.m_layout = layout;
}

void VCmdBuffer::CopyImage(VImage &src, VImage &dst)
{
  VkImageCopy copy;
  copy.srcSubresource = { VImage::AspectFromFormat(src.m_format), 0, 0, 1 };
  copy.srcOffset = { 0, 0, 0 };
  copy.dstSubresource = { VImage::AspectFromFormat(dst.m_format), 0, 0, 1 };
  copy.dstOffset = { 0, 0, 0 };
  copy.extent = { std::min(src.m_size.width, dst.m_size.width), std::min(src.m_size.height, dst.m_size.height), std::min(src.m_size.depth, dst.m_size.depth) };

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdCopyImage(m_buffer, src.m_image, src.m_layout, dst.m_image, dst.m_layout, 1, &copy);
}

void VCmdBuffer::CopyBuffer(VBuffer &src, VBuffer &dst, uint64_t srcOffset, uint64_t dstOffset, uint64_t size)
{
  VkBufferCopy copy;
  copy.size = std::min(size, std::min(src.m_size - srcOffset, dst.m_size - dstOffset));
  copy.srcOffset = srcOffset;
  copy.dstOffset = dstOffset;

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdCopyBuffer(m_buffer, src.m_buffer, dst.m_buffer, 1, &copy);
}

void VCmdBuffer::SetViewport(VkViewport &viewport)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdSetViewport(m_buffer, 0, 1, &viewport);
}

void VCmdBuffer::BindPipeline(VGraphicsPipeline &pipeline)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdBindPipeline(m_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_pipeline);
}

void VCmdBuffer::BindDescriptorSets(VPipelineLayout &pipelineLayout, std::vector<std::unique_ptr<VDescriptorSet>> const &sets, VkPipelineBindPoint bindPoint)
{
  std::vector<VkDescriptorSet> vkSets;
  for (auto &set : sets) 
    vkSets.push_back(set->m_set);

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdBindDescriptorSets(m_buffer, bindPoint, pipelineLayout.m_layout, 0, (uint32_t)vkSets.size(), vkSets.data(), 0, nullptr);
}

void VCmdBuffer::BindVertexBuffers(std::vector<std::shared_ptr<VVertexBuffer>> &vertexBuffers)
{
  std::vector<VkBuffer> vkBuffers;
  for (auto &buf : vertexBuffers)
    vkBuffers.push_back(buf->m_buffer->m_buffer);

  std::vector<VkDeviceSize> offsets;
  offsets.assign(vkBuffers.size(), 0);

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdBindVertexBuffers(m_buffer, 0, (uint32_t)vkBuffers.size(), vkBuffers.data(), offsets.data());
}

void VCmdBuffer::BindIndexBuffer(VIndexBuffer &indexBuffer, VkDeviceSize offset)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdBindIndexBuffer(m_buffer, indexBuffer.m_buffer->m_buffer, offset, indexBuffer.m_type);
}

void VCmdBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdDrawIndexed(m_buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VCmdBuffer::ExecuteCommands(VCmdBuffer &cmdBuffer)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdExecuteCommands(m_buffer, 1, &cmdBuffer.m_buffer);
}

void VCmdBuffer::ExecuteCommands(std::vector<VCmdBuffer*> const &cmdBuffers)
{
  std::vector<VkCommandBuffer> vkBuffers;
  for (auto buf : cmdBuffers)
    vkBuffers.push_back(buf->m_buffer);

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdExecuteCommands(m_buffer, (uint32_t)vkBuffers.size(), vkBuffers.data());
}


void VCmdBuffer::BeginRenderPass(VRenderPass &pass, VFramebuffer &framebuffer, uint32_t width, uint32_t height, std::vector<VkClearValue> const &clearValues, bool secondaryBuffers)
{
  VkRenderPassBeginInfo passBegin;
  passBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  passBegin.pNext = nullptr;
  passBegin.renderPass = pass.m_renderPass;
  passBegin.framebuffer = framebuffer.m_framebuffer;
  passBegin.renderArea = { { 0, 0 }, { width, height } };
  passBegin.clearValueCount = (uint32_t)clearValues.size();
  passBegin.pClearValues = clearValues.data();

  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdBeginRenderPass(m_buffer, &passBegin, secondaryBuffers ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);
}

void VCmdBuffer::EndRenderPass()
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdEndRenderPass(m_buffer);
}

void VCmdBuffer::NextSubpass(bool secondaryBuffers)
{
  std::lock_guard<ConditionalLock> lock(m_pool->m_lock);
  AutoBegin();

  vkCmdNextSubpass(m_buffer, secondaryBuffers ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);
}

VQueue::VQueue(VDevice &device, bool synchronize, VkQueue queue): m_queue(queue), m_device(&device), m_lock(synchronize)
{
}

void VQueue::Submit(VCmdBuffer &cmdBuffer, std::vector<VSemaphore*>* waitSemaphores)
{
  cmdBuffer.AutoEnd();

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
    throw VGraphicsException("VQueue::Submit failed in vkQueueSubmit", err);
}

void VQueue::WaitIdle()
{
  VkResult err = vkQueueWaitIdle(m_queue);
  if (err < 0)
    throw VGraphicsException("VQueue::WaitIdle failed in vkQueueWaitIdle", err);
}

VDevice::VDevice(VGraphics &graphics) :
  m_graphics(&graphics),
  m_device(VK_NULL_HANDLE, [](VkDevice &device) { vkDestroyDevice(device, nullptr); }, false),
  m_clearValues(2)
{
  InitCapabilities();
  InitDevice();
  InitCmdBuffers(true, 1);
  m_swapchain.reset(new VSwapchain(*this));
  InitDepth();
  SubmitInitCommands();
  m_renderPass.reset(new VRenderPass(*this));
  m_imageAvailable.reset(new VSemaphore(*this, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));
  InitFramebuffers();
  m_pipelineCache.reset(new VPipelineCache(*this));
  m_descriptorPool.reset(new VDescriptorPool(*this, true, 256));
  InitViewportState();
  m_multisampleState = std::make_shared<VMultisampleState>();
  m_dynamicState = std::make_shared<VDynamicState>();
  SetClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  SetClearDepthStencil(1.0f, 0);
}

VDevice::~VDevice()
{
  WaitIdle();
}

void VDevice::InitCapabilities()
{
  std::vector<VkLayerProperties> layers;
  VkResult err = AppendData<VkLayerProperties>(layers, [this](uint32_t &s, VkLayerProperties *p)->VkResult { return vkEnumerateDeviceLayerProperties(m_graphics->m_physicalDevice, &s, p); });
  if (err < 0)
    throw VGraphicsException("VDevice::InitCapabilities failed in vkEnumerateDeviceLayerProperties", err);

  m_layerNames.clear();
  if (m_graphics->m_validate) {
    if (!VGraphics::AddLayerName(layers, "VK_LAYER_LUNARG_standard_validation", m_layerNames))
      throw VGraphicsException("VDevice::InitCapabilities failed to add device validation layer", err);
  }

  std::vector<VkExtensionProperties> extensions;
  err = AppendData<VkExtensionProperties>(extensions, [this](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateDeviceExtensionProperties(m_graphics->m_physicalDevice, nullptr, &s, p); });
  if (err < 0)
    throw VGraphicsException("VDevice::InitCapabilities failed in vkEnumerateDeviceExtensionProperties", err);
  for (auto &l : m_layerNames) {
    err = AppendData<VkExtensionProperties>(extensions, [this, &l](uint32_t &s, VkExtensionProperties *p)->VkResult { return vkEnumerateDeviceExtensionProperties(m_graphics->m_physicalDevice, l.c_str(), &s, p); });
    if (err < 0)
      throw VGraphicsException("VDevice::InitCapabilities failed in vkEnumerateDeviceExtensionProperties", err);
  }

  m_extensionNames.clear();
  if (!VGraphics::AddExtensionName(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME, m_extensionNames))
    throw VGraphicsException("VDevice::InitCapabilities failed to add VK_KHR_SWAPCHAIN_EXTENSION_NAME extension", err);
}

void VDevice::InitDevice()
{
  VkResult err;
  VkDeviceQueueCreateInfo queueInfo;
  float queuePriorities[] = { 0.0 };
  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfo.pNext = nullptr;
  queueInfo.flags = 0;
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

  VkDevice device = VK_NULL_HANDLE;
  err = vkCreateDevice(m_graphics->m_physicalDevice, &deviceInfo, nullptr, &device);
  if (err < 0)
    throw VGraphicsException("VDevice::InitDevice failed in vkCreateDevice", err);
  m_device.Reset(std::move(device));

  VkQueue queue;
  vkGetDeviceQueue(m_device, m_graphics->m_graphicsQueueFamily, 0, &queue);
  m_queue.reset(new VQueue(*this, true, queue));

  GET_DEVICE_PROC_ADDRESS(CreateSwapchainKHR);
  GET_DEVICE_PROC_ADDRESS(DestroySwapchainKHR);
  GET_DEVICE_PROC_ADDRESS(GetSwapchainImagesKHR);
  GET_DEVICE_PROC_ADDRESS(AcquireNextImageKHR);
  GET_DEVICE_PROC_ADDRESS(QueuePresentKHR);
}

void VDevice::InitDepth()
{
  VkSurfaceCapabilitiesKHR surfaceCaps;
  VkResult err = m_graphics->m_GetPhysicalDeviceSurfaceCapabilitiesKHR(m_graphics->m_physicalDevice, m_graphics->m_surface, &surfaceCaps);
  if (err < 0)
    throw VGraphicsException("VDevice::InitDepth failed in GetPhysicalDeviceSurfaceCapabilitiesKHR", err);

  m_depth.reset(new VImage(*this, true, VK_FORMAT_D24_UNORM_S8_UINT, surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height));

  m_cmdInit->SetImageLayout(*m_depth, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
}

void VDevice::InitCmdBuffers(bool synchronize, uint32_t count)
{
  m_cmdPool.reset(new VCmdPool(*this, synchronize, false, true));
  m_cmdInit.reset(m_cmdPool->CreateBuffer(true));
  m_cmdFrame.reset(m_cmdPool->CreateBuffer(true));
  m_cmdFrame->m_autoBegin = false;
  m_cmdFrame->SetUseSemaphore(true, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

void VDevice::InitFramebuffers()
{
  for (auto &img : m_swapchain->m_images)
    m_framebuffers.push_back(std::make_unique<VFramebuffer>(m_renderPass.get(), std::vector<VImage*>{ img.get(), m_depth.get() }));
}

void VDevice::InitViewportState()
{
  VkExtent3D &size = m_swapchain->m_images[0]->m_size;
  m_viewportState = std::make_shared<VViewportState>(size.width, size.height);
}

void VDevice::SubmitInitCommands()
{
  m_queue->Submit(*m_cmdInit);
  m_queue->WaitIdle();
  m_cmdInit->Reset();
  m_cmdInit->Begin();
}

VImage *VDevice::LoadVImage(std::string const &filename)
{
  uint32_t width, height, depth, components;
  try {
    cimg_library::cimg::exception_mode(1);
    cimg_library::CImg<unsigned char> image(filename.c_str());
    width = image.width(); 
    height = image.height(); 
    depth = image.depth(); 
    components = image.spectrum();

    UpdateStagingImage(width, height, depth, components);

    cimg_library::CImg<unsigned char> imgRGBA;
    
    if (components < 4) {
      imgRGBA = image.get_channels(0, 3);
      if (components < 2)
        imgRGBA.get_shared_channels(components + 1, 2) = static_cast<unsigned char>(0);
      imgRGBA.get_shared_channel(3) = 255;
    } else
      imgRGBA = image.get_shared_channels(0, 3);

    imgRGBA.permute_axes("cxyz");

    void *mem = m_stagingImage->Map();
    memcpy(mem, imgRGBA.data(), imgRGBA.size());
    m_stagingImage->Unmap();

  } catch (cimg_library::CImgException &) {
    return nullptr;
  }

  VkFormat format = VImage::FormatFromComponents(components);
  std::unique_ptr<VImage> vimg(new VImage(*this, true, format, width, height, depth));

  if (m_stagingImage->m_layout != VK_IMAGE_LAYOUT_GENERAL)
    m_cmdInit->SetImageLayout(*m_stagingImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);

  m_cmdInit->SetImageLayout(*vimg, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
  m_cmdInit->CopyImage(*m_stagingImage, *vimg);
  m_cmdInit->SetImageLayout(*vimg, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
  m_cmdInit->SetImageLayout(*m_stagingImage, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_HOST_WRITE_BIT);

  SubmitInitCommands();

  return vimg.release();
}

VBuffer *VDevice::LoadVBuffer(uint64_t size, VkBufferUsageFlags usage, void *data)
{
  std::unique_ptr<VBuffer> buffer = std::make_unique<VBuffer>(*this, false, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, false);
  if (data) {
    UpdateStagingBuffer(size);

    void *stagingMem = m_stagingBuffer->Map();
    memcpy(stagingMem, data, size);
    m_stagingBuffer->Unmap();

    m_cmdInit->CopyBuffer(*m_stagingBuffer, *buffer);
    SubmitInitCommands();
  }
  return buffer.release();
}

VShader *VDevice::LoadVShader(std::string const &filename)
{
  std::ifstream file(filename, std::ios::binary);
  std::vector<char> code;
  code.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  if (code.size() == 0 || code.size() % 4 != 0)
    throw VGraphicsException("VDevice::LoadVShader failed because loaded shader code is invalid", VK_RESULT_MAX_ENUM);

  VkShaderStageFlagBits stageBits = {};
  if (filename.find(".vert") != std::string::npos)
    stageBits = VK_SHADER_STAGE_VERTEX_BIT;
  else if (filename.find(".frag") != std::string::npos)
    stageBits = VK_SHADER_STAGE_FRAGMENT_BIT;
  else if (filename.find(".comp") != std::string::npos)
    stageBits = VK_SHADER_STAGE_COMPUTE_BIT;
  else if (filename.find(".geom") != std::string::npos)
    stageBits = VK_SHADER_STAGE_GEOMETRY_BIT;
  else if (filename.find(".tesc") != std::string::npos)
    stageBits = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  else if (filename.find(".tese") != std::string::npos)
    stageBits = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  if (!stageBits)
    throw VGraphicsException("VDevice::LoadVShader failed to recongize shader file extension", VK_RESULT_MAX_ENUM);

  VShader *shader = new VShader(*this, stageBits, code.size(), (uint32_t*)code.data());
  return shader;
}

void VDevice::UpdateStagingImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t components)
{
  if (!m_stagingImage || m_stagingImage->m_size.width < width || m_stagingImage->m_size.height < height || m_stagingImage->m_size.depth < depth)
    m_stagingImage.reset(new VImage(*this, true, VK_FORMAT_R8G8B8A8_UNORM, width, height, depth, 1, 1, true));
}

void VDevice::FreeStagingImage()
{
  m_stagingImage.reset();
}

void VDevice::UpdateStagingBuffer(uint64_t size)
{
  if (!m_stagingBuffer || m_stagingBuffer->m_size < size)
    m_stagingBuffer.reset(new VBuffer(*this, true, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true));
}

void VDevice::FreeStagingBuffer()
{
  m_stagingBuffer.reset();
}

void VDevice::SetClearColor(float r, float g, float b, float a)
{
  m_clearValues[0].color = { r, g, b, a };
}

void VDevice::SetClearDepthStencil(float depth, uint32_t stencil)
{
  m_clearValues[1].depthStencil = { depth, stencil };
}

void VDevice::Add(std::shared_ptr<VModelInstance> instance)
{
  m_toRender.push_back(std::move(instance));
}

void VDevice::RenderFrame()
{
  uint32_t imageIndex;
  VkResult err = m_AcquireNextImageKHR(m_device, m_swapchain->m_swapchain, (uint64_t)-1, m_imageAvailable->m_semaphore, VK_NULL_HANDLE, &imageIndex);
  if (err < 0)
    throw VGraphicsException("VDevice::RenderFrame failed in vkAcquireNextImageKHR", err);

  m_queue->WaitIdle();
  
  m_cmdFrame->Reset();
  m_cmdFrame->Begin(false, m_renderPass.get(), 0);
  m_cmdFrame->BeginRenderPass(*m_renderPass, *m_framebuffers[imageIndex], m_swapchain->m_images[0]->m_size.width, m_swapchain->m_images[0]->m_size.height, m_clearValues, true);

  for (auto &inst : m_toRender)
    m_cmdFrame->ExecuteCommands(*inst->m_cmdBuffer);
  m_toRender.clear();

  m_cmdFrame->EndRenderPass();
  m_cmdFrame->End();

  std::vector<VSemaphore*> sem{ m_imageAvailable.get() };
  m_queue->Submit(*m_cmdFrame, &sem);

  VkPresentInfoKHR presentInfo;
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = 0;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &m_cmdFrame->m_semaphore->m_semaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &m_swapchain->m_swapchain.Get();
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;

  err = m_QueuePresentKHR(m_queue->m_queue, &presentInfo);
  if (err < 0)
    throw VGraphicsException("VDevice::RenderFrame failed in vkQueuePresentKHR", err);
}

void VDevice::WaitIdle()
{
  VkResult err = vkDeviceWaitIdle(m_device);
  if (err < 0)
    throw VGraphicsException("VDevice::WaitIdle failed in vkDeviceWaitIdle", err);
}

VSwapchain::VSwapchain(VDevice &device) : 
  m_device(&device), 
  m_swapchain(VK_NULL_HANDLE, [this](VkSwapchainKHR& swapchain) {m_device->m_DestroySwapchainKHR(m_device->m_device, swapchain, nullptr);}, false)
{
  VkSurfaceCapabilitiesKHR surfaceCaps;
  InitSwapchain(surfaceCaps);
  InitImages(surfaceCaps);
}

void VSwapchain::InitSwapchain(VkSurfaceCapabilitiesKHR &surfaceCaps)
{
  VkResult err;
  err = m_device->m_graphics->m_GetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->m_graphics->m_physicalDevice, m_device->m_graphics->m_surface, &surfaceCaps);
  if (err < 0)
    throw VGraphicsException("VSwapchain::InitSwapchain failed in GetPhysicalDeviceSurfaceCapabilitiesKHR", err);

  std::vector<VkPresentModeKHR> presentModes;
  err = AppendData<VkPresentModeKHR>(presentModes, [this](uint32_t &s, VkPresentModeKHR *m)->VkResult { return m_device->m_graphics->m_GetPhysicalDeviceSurfacePresentModesKHR(m_device->m_graphics->m_physicalDevice, m_device->m_graphics->m_surface, &s, m); });
  if (err < 0)
    throw VGraphicsException("VSwapchain::InitSwapchain failed in GetPhysicalDeviceSurfacePresentModesKHR", err);

  VkPresentModeKHR mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
  for (auto m : { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR }) {
    if (std::find(presentModes.begin(), presentModes.end(), m) != presentModes.end()) {
      mode = m;
      break;
    }
  }
  if (mode == VK_PRESENT_MODE_MAX_ENUM_KHR)
    throw VGraphicsException("VSwapchain::InitSwapchain failed to find a suitable present mode", VK_RESULT_MAX_ENUM);

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
  chainInfo.oldSwapchain = VK_NULL_HANDLE;

  VkSwapchainKHR swapchain;
  err = m_device->m_CreateSwapchainKHR(m_device->m_device, &chainInfo, nullptr, &swapchain);
  if (err < 0)
    throw VGraphicsException("VSwapchain::InitSwapchain failed in CreateSwapchainKHR", err);
  m_swapchain.Reset(std::move(swapchain));
}

void VSwapchain::InitImages(VkSurfaceCapabilitiesKHR &surfaceCaps)
{
  std::vector<VkImage> images;
  VkResult err = AppendData<VkImage>(images, [this](uint32_t &s, VkImage *i)->VkResult { return m_device->m_GetSwapchainImagesKHR(m_device->m_device, m_swapchain, &s, i); });
  if (err < 0)
    throw VGraphicsException("VSwapchain::InitImages failed in GetSwapchainImagesKHR", err);

  for (auto im : images) {
    VImage *img = new VImage(*m_device, im, m_device->m_graphics->m_surfaceFormat.format, surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height, 1);
    m_images.push_back(std::unique_ptr<VImage>(img));
    m_device->m_cmdInit->SetImageLayout(*img, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, VK_ACCESS_MEMORY_READ_BIT);
  }
}

VFramebuffer::VFramebuffer(VRenderPass *renderPass, std::vector<VImage*> attachments) :
  m_device(attachments[0]->m_device)
{
  std::vector<VkImageView> views;
  for (auto img : attachments)
    views.push_back(img->m_view);

  VkFramebufferCreateInfo fbInfo;
  fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fbInfo.pNext = nullptr;
  fbInfo.flags = 0;
  fbInfo.renderPass = renderPass->m_renderPass;
  fbInfo.attachmentCount = (uint32_t)views.size();
  fbInfo.pAttachments = views.data();
  fbInfo.width = attachments[0]->m_size.width;
  fbInfo.height = attachments[0]->m_size.height;
  fbInfo.layers = 1;

  VkResult err = vkCreateFramebuffer(m_device->m_device, &fbInfo, nullptr, &m_framebuffer);
  if (err < 0)
    throw VGraphicsException("VFramebuffer failed in vkCreateFramebuffer", err);
}

VFramebuffer::~VFramebuffer()
{
  vkDestroyFramebuffer(m_device->m_device, m_framebuffer, nullptr);
}


VImage::VImage(VDevice &device, bool synchronize, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLayers, bool linear) :
  m_device(&device),
  m_image(VK_NULL_HANDLE, [this](VkImage &image) { vkDestroyImage(m_device->m_device, image, nullptr); }, false),
  m_view(VK_NULL_HANDLE, [this](VkImageView &view) { vkDestroyImageView(m_device->m_device, view, nullptr); }, false)
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
  imgInfo.usage = linear ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : UsageFromFormat(format);
  imgInfo.tiling = linear ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
  imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imgInfo.initialLayout = m_layout;
  imgInfo.queueFamilyIndexCount = 0;
  imgInfo.pQueueFamilyIndices = nullptr;

  VkImage image = VK_NULL_HANDLE;
  VkResult err = vkCreateImage(m_device->m_device, &imgInfo, nullptr, &image);
  if (err < 0)
    throw VGraphicsException("VImage failed in vkCreateImage", err);
  m_image.Reset(std::move(image));

  VkMemoryRequirements imgMem;
  vkGetImageMemoryRequirements(m_device->m_device, m_image, &imgMem);

  m_memory = std::make_shared<VMemory>(*m_device, synchronize, imgMem.size, imgMem.memoryTypeBits, linear ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0);

  err = vkBindImageMemory(m_device->m_device, m_image, (*m_memory).m_memory, 0);
  if (err < 0)
    throw VGraphicsException("VImage failed in vkBindImageMemory", err);

  if (!linear)
    Init(VK_NULL_HANDLE, format, ViewTypeFromDimensions(width, height, depth, arrayLayers));
}

VImage::VImage(VDevice &device, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, VkImageViewType imgType) :
  m_device(&device),
  m_size{width, height, depth},
  m_image(VK_NULL_HANDLE, [this](VkImage &image) { vkDestroyImage(m_device->m_device, image, nullptr); }, false),
  m_view(VK_NULL_HANDLE, [this](VkImageView &view) { vkDestroyImageView(m_device->m_device, view, nullptr); }, false)
{
  Init(image, format, imgType);
}

void VImage::Init(VkImage image, VkFormat format, VkImageViewType imgType)
{
  if (image) {
    assert(!m_memory);
    m_image.Reset(std::move(image));
    m_image.Release(); // do not delete in image we were passed on construction, we do not own that
  }
  m_format = format;

  VkImageViewCreateInfo viewInfo;
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.pNext = nullptr;
  viewInfo.flags = 0;
  viewInfo.image = m_image;
  viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
  viewInfo.format = format;
  viewInfo.viewType = imgType;
  viewInfo.subresourceRange = { AspectFromFormat(format), 0, 1, 0, 1 };

  VkImageView view = VK_NULL_HANDLE;
  VkResult err = vkCreateImageView(m_device->m_device, &viewInfo, nullptr, &view);
  if (err < 0)
    throw VGraphicsException("VImage failed in vkCreateImageView", err);
  m_view.Reset(std::move(view));
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

void *VImage::Map()
{
  return (*m_memory).Map();
}

void VImage::Unmap()
{
  (*m_memory).Unmap();
}

VSemaphore::VSemaphore(VDevice &device, VkPipelineStageFlags stages) : m_device(&device), m_stages(stages)
{
  VkSemaphoreCreateInfo semInfo;
  semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  semInfo.pNext = nullptr;
  semInfo.flags = 0;

  VkResult err = vkCreateSemaphore(m_device->m_device, &semInfo, nullptr, &m_semaphore);
  if (err < 0)
    throw VGraphicsException("VSemaphore failed in vkCreateSemaphore", err);
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
  if (err < 0)
    throw VGraphicsException("VFence failed in vkCreateFence", err);
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

VSampler::VSampler(VDevice &device, VkSamplerAddressMode addressMode, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, float maxAnisotropy): m_device(&device)
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
    throw VGraphicsException("VSampler failed in vkCreateSampler", err);
}

VSampler::~VSampler()
{
  vkDestroySampler(m_device->m_device, m_sampler, nullptr);
}

VMemory::VMemory(VDevice &device, bool synchronize, uint64_t size, uint32_t validMemoryTypes, VkMemoryPropertyFlags memFlags) : 
  m_device(&device),
  m_lock(synchronize)
{
  VkMemoryAllocateInfo memInfo;
  memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memInfo.pNext = nullptr;
  memInfo.allocationSize = size;
  memInfo.memoryTypeIndex = m_device->m_graphics->GetMemoryTypeIndex(validMemoryTypes, memFlags);
  VkResult err = vkAllocateMemory(m_device->m_device, &memInfo, nullptr, &m_memory);
  if (err < 0)
    throw VGraphicsException("VMemory: failed in vkAllocateMemory", err);
  m_size = size;
}

VMemory::~VMemory()
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  vkFreeMemory(m_device->m_device, m_memory, nullptr);
}

void *VMemory::Map(uint64_t offset, uint64_t size)
{
  void *mapped;
  std::lock_guard<ConditionalLock> lock(m_lock);
  VkResult err = vkMapMemory(m_device->m_device, m_memory, offset, size, 0, &mapped);
  if (err < 0)
    throw VGraphicsException("VMemory::Map failed in vkMapMemory", err);
  return mapped;
}

void VMemory::Unmap()
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  vkUnmapMemory(m_device->m_device, m_memory);
}

VBuffer::VBuffer(VDevice &device, bool synchronize, uint64_t size, VkBufferUsageFlags usage, bool hostVisible) : 
  m_device(&device),
  m_buffer(VK_NULL_HANDLE, [this](VkBuffer &buffer) { vkDestroyBuffer(m_device->m_device, buffer, nullptr); }, false)
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

  VkBuffer buffer = VK_NULL_HANDLE;
  VkResult err = vkCreateBuffer(m_device->m_device, &bufInfo, nullptr, &buffer);
  if (err < 0)
    throw VGraphicsException("VBuffer failed in vkCreateBuffer", err);
  m_buffer.Reset(std::move(buffer));

  VkMemoryRequirements memReqs;
  vkGetBufferMemoryRequirements(m_device->m_device, m_buffer, &memReqs);

  m_memory = std::make_shared<VMemory>(*m_device, true, memReqs.size, memReqs.memoryTypeBits, hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0);

  err = vkBindBufferMemory(m_device->m_device, m_buffer, (*m_memory).m_memory, 0);
  if (err < 0)
    throw VGraphicsException("VBuffer failed in vkBindBufferMemory", err);

  // The following usages require a buffer view, we don't care for them now
  assert(!(m_usage & (VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)));
}

void *VBuffer::Map()
{
  return (*m_memory).Map();
}

void VBuffer::Unmap()
{
  (*m_memory).Unmap();
}

VShader::VShader(VDevice &device, VkShaderStageFlagBits stageBits, size_t size, uint32_t *code): 
  m_device(&device),
  m_stageBits(stageBits)
{
  VkShaderModuleCreateInfo shaderInfo;
  shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderInfo.pNext = nullptr;
  shaderInfo.flags = 0;
  shaderInfo.codeSize = size;
  shaderInfo.pCode = code;

  VkResult err = vkCreateShaderModule(m_device->m_device, &shaderInfo, nullptr, &m_shader);
  if (err < 0)
    throw VGraphicsException("VShader failed in vkCreateShaderModule", err);
}

VShader::~VShader()
{
  vkDestroyShaderModule(m_device->m_device, m_shader, nullptr);
}

void VShader::FillPipelineInfo(VkPipelineShaderStageCreateInfo &info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.module = m_shader;
  info.stage = m_stageBits;
  info.pName = "main";
  info.pSpecializationInfo = nullptr;
}

VDescriptorSetLayout::VDescriptorSetLayout(VDevice &device) :
  m_device(&device)
{
  VkDescriptorSetLayoutBinding uniformBinding;
  uniformBinding.binding = 0;
  uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniformBinding.descriptorCount = 1;
  uniformBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
  uniformBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layout;
  layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout.pNext = nullptr;
  layout.flags = 0;
  layout.bindingCount = 1;
  layout.pBindings = &uniformBinding;

  VkResult err = vkCreateDescriptorSetLayout(m_device->m_device, &layout, nullptr, &m_layout);
  if (err < 0)
    throw VGraphicsException("VDescriptorSetLayout failed in vkCreateDescriptorSetLayout", err);
}

VDescriptorSetLayout::~VDescriptorSetLayout()
{
  vkDestroyDescriptorSetLayout(m_device->m_device, m_layout, nullptr);
}

VPipelineLayout::VPipelineLayout(VDevice &device, std::vector<std::shared_ptr<VDescriptorSetLayout>> const &setLayouts) :
  m_device(&device),
  m_setLayouts(setLayouts.begin(), setLayouts.end())
{
  std::vector<VkDescriptorSetLayout> vkSetLayouts(m_setLayouts.size());
  for (int i = 0; i < m_setLayouts.size(); ++i)
    vkSetLayouts[i] = m_setLayouts[i]->m_layout;

  VkPipelineLayoutCreateInfo layoutInfo;
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.pNext = nullptr;
  layoutInfo.flags = 0;
  layoutInfo.setLayoutCount = (uint32_t)vkSetLayouts.size();
  layoutInfo.pSetLayouts = vkSetLayouts.data();
  layoutInfo.pushConstantRangeCount = 0;
  layoutInfo.pPushConstantRanges = nullptr;

  VkResult err = vkCreatePipelineLayout(m_device->m_device, &layoutInfo, nullptr, &m_layout);
  if (err < 0)
    throw VGraphicsException("VPipelineLayout failed in vkCreatePipelineLayout", err);
}

VPipelineLayout::~VPipelineLayout()
{
  vkDestroyPipelineLayout(m_device->m_device, m_layout, nullptr);
}

void VVertexInfo::AddAttribute(VkFormat format, uint32_t offset)
{
  AddAttribute(0, m_attr, format, offset);
}

void VVertexInfo::FillPipelineInfo(uint32_t binding, std::vector<VkVertexInputAttributeDescription> &attr, std::vector<VkVertexInputBindingDescription> &bindings)
{
  assert(m_stride > 0);
  VkVertexInputBindingDescription bind;

  bind.binding = binding;
  bind.inputRate = m_rate;
  bind.stride = m_stride;
  bindings.push_back(bind);

  for (auto &desc : m_attr)
    AddAttribute(binding, attr, desc.format, desc.offset);
}

void VVertexInfo::AddAttribute(uint32_t binding, std::vector<VkVertexInputAttributeDescription> &attr, VkFormat format, uint32_t offset)
{
  attr.resize(attr.size() + 1);
  attr.back().binding = binding;
  attr.back().location = attr.size() == 1 ? 0 : attr[attr.size() - 2].location + 1;
  attr.back().format = format;
  attr.back().offset = offset;
}


VVertexBuffer::VVertexBuffer(VDevice &device, uint64_t size, void *data, std::shared_ptr<VVertexInfo> vertexInfo) :
  m_vertexInfo(std::move(vertexInfo)),
  m_buffer(device.LoadVBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data))
{
}

VIndexBuffer::VIndexBuffer(VDevice &device, uint64_t count, uint16_t *indices) :
  m_type(VK_INDEX_TYPE_UINT16),
  m_buffer(device.LoadVBuffer(count * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices))
{
}

VIndexBuffer::VIndexBuffer(VDevice &device, uint64_t count, uint32_t *indices) :
  m_type(VK_INDEX_TYPE_UINT32),
  m_buffer(device.LoadVBuffer(count * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices))
{
}

uint32_t VIndexBuffer::TypeSize(VkIndexType type)
{
  switch (type) {
    case VK_INDEX_TYPE_UINT16:
      return sizeof(uint16_t);
    case VK_INDEX_TYPE_UINT32:
      return sizeof(uint32_t);
    default:
      assert(!"Invalid index type");
      return 0;
  }
}

VMaterial::VMaterial(std::vector<std::shared_ptr<VShader>> const &shaders) :
  m_shaders(shaders),
  m_blendState(std::make_shared<VBlendState>(VBlendState::DISABLED)),
  m_depthStencilState(std::make_shared<VDepthStencilState>()),
  m_rasterizationState(std::make_shared<VRasterizationState>()),
  m_multisampleState(GetDevice().m_multisampleState),
  m_viewportState(GetDevice().m_viewportState),
  m_dynamicState(GetDevice().m_dynamicState)
{
  CreatePipelineLayout();
}

void VMaterial::CreatePipelineLayout()
{
  std::vector<std::shared_ptr<VDescriptorSetLayout>> descriptorSets;
  // just one set with a single uniform buffer shared by all stages
  descriptorSets.push_back(std::make_shared<VDescriptorSetLayout>(GetDevice()));
  m_pipelineLayout.reset(new VPipelineLayout(GetDevice(), descriptorSets));
}

void VMaterial::SetDynamicState(VCmdBuffer *cmdBuffer)
{
  for (auto state : m_dynamicState->m_states) {
    switch (state) {
      case VK_DYNAMIC_STATE_VIEWPORT:
        cmdBuffer->SetViewport(m_viewportState->m_viewport);
        break;
      default:
        assert(!"Unsupported dynamic state");
    }
  }
}

VGeometry::VGeometry(VkPrimitiveTopology topology, std::shared_ptr<VIndexBuffer> indexBuffer, std::vector<std::shared_ptr<VVertexBuffer>> &vertexBuffers) :
  m_topology(topology),
  m_indexBuffer(indexBuffer),
  m_vertexBuffers(vertexBuffers)
{
}

void VGeometry::FillPipelineInfo(VkPipelineInputAssemblyStateCreateInfo &info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.topology = m_topology;
  info.primitiveRestartEnable = false;
}

VModel::VModel(std::shared_ptr<VGeometry> geometry, std::shared_ptr<VMaterial> material) :
  m_geometry(std::move(geometry)),
  m_material(std::move(material))
{
  CreatePipeline();
}

void VModel::CreatePipeline(uint32_t subpass)
{
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  shaderStages.resize(m_material->m_shaders.size());
  for (int i = 0; i < shaderStages.size(); ++i)
    m_material->m_shaders[i]->FillPipelineInfo(shaderStages[i]);

  std::vector<VkVertexInputAttributeDescription> vertexAttr;
  std::vector<VkVertexInputBindingDescription> vertexBindings;
  for (uint32_t i = 0; i < m_geometry->m_vertexBuffers.size(); ++i)
    m_geometry->m_vertexBuffers[i]->m_vertexInfo->FillPipelineInfo(i, vertexAttr, vertexBindings);

  VkPipelineVertexInputStateCreateInfo vertexInput;
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.pNext = nullptr;
  vertexInput.flags = 0;
  vertexInput.vertexBindingDescriptionCount = (uint32_t)vertexBindings.size();
  vertexInput.pVertexBindingDescriptions = vertexBindings.data();
  vertexInput.vertexAttributeDescriptionCount = (uint32_t)vertexAttr.size();
  vertexInput.pVertexAttributeDescriptions = vertexAttr.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  m_geometry->FillPipelineInfo(inputAssembly);

  VkPipelineViewportStateCreateInfo viewportState;
  m_material->m_viewportState->FillPilelineInfo(viewportState);

  VkPipelineRasterizationStateCreateInfo rasterizationState;
  m_material->m_rasterizationState->FillPipelineInfo(rasterizationState);

  VkPipelineMultisampleStateCreateInfo multisampleState;
  m_material->m_multisampleState->FillPipelineState(multisampleState);

  VkPipelineDepthStencilStateCreateInfo depthStencilState;
  m_material->m_depthStencilState->FillPipelineState(depthStencilState);

  VkPipelineColorBlendStateCreateInfo blendState;
  m_material->m_blendState->FillPipelineState(blendState);

  VkPipelineDynamicStateCreateInfo dynamicState;
  m_material->m_dynamicState->FillPipelineState(dynamicState);

  VDevice &device = m_geometry->GetDevice();

  VkGraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.pNext = nullptr;
  pipelineInfo.flags = 0;
  pipelineInfo.stageCount = (uint32_t)shaderStages.size();
  pipelineInfo.pStages = shaderStages.data();
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pTessellationState = nullptr;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizationState;
  pipelineInfo.pMultisampleState = &multisampleState;
  pipelineInfo.pDepthStencilState = &depthStencilState;
  pipelineInfo.pColorBlendState = &blendState;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_material->m_pipelineLayout->m_layout;
  pipelineInfo.renderPass = device.m_renderPass->m_renderPass;
  pipelineInfo.subpass = subpass;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  VkPipeline pipeline = VK_NULL_HANDLE;
  VkResult err = vkCreateGraphicsPipelines(device.m_device, device.m_pipelineCache->m_cache, 1, &pipelineInfo, nullptr, &pipeline);
  if (err < 0)
    throw VGraphicsException("VModel::CreatePipeline failed in vkCreateGraphicsPipelines", err);

  m_pipeline.reset(new VGraphicsPipeline(*device.m_pipelineCache, pipeline));
}

VViewportState::VViewportState(uint32_t width, uint32_t height)
{
  SetSize(width, height);
}

void VViewportState::SetSize(uint32_t width, uint32_t height)
{
  m_viewport.x = 0.0f;
  m_viewport.y = 0.0f;
  m_viewport.width = (float)width;
  m_viewport.height = (float)height;
  m_viewport.minDepth = 0.0f;
  m_viewport.maxDepth = 1.0f;

  m_scissor.offset = { 0, 0 };
  m_scissor.extent = { width, height };
}

void VViewportState::FillPilelineInfo(VkPipelineViewportStateCreateInfo &info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.viewportCount = 1;
  info.pViewports = &m_viewport;
  info.scissorCount = 1;
  info.pScissors = &m_scissor;
}

VRasterizationState::VRasterizationState()
{
  m_rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_rasterization.pNext = nullptr;
  m_rasterization.flags = 0;
  m_rasterization.depthClampEnable = VK_FALSE;
  m_rasterization.rasterizerDiscardEnable = VK_FALSE;
  m_rasterization.polygonMode = VK_POLYGON_MODE_FILL;
  m_rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
  m_rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  m_rasterization.depthBiasEnable = VK_FALSE;
  m_rasterization.depthBiasConstantFactor = 0.0f;
  m_rasterization.depthBiasClamp = 0.0f;
  m_rasterization.depthBiasSlopeFactor = 0.0f;
  m_rasterization.lineWidth = 1.0f;
}

void VRasterizationState::FillPipelineInfo(VkPipelineRasterizationStateCreateInfo &info)
{
  info = m_rasterization;
}

VMultisampleState::VMultisampleState()
{
  m_multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_multisample.pNext = nullptr;
  m_multisample.flags = 0;
  m_multisample.sampleShadingEnable = VK_FALSE;
  m_multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_multisample.minSampleShading = 1.0f;
  m_multisample.pSampleMask = nullptr;
  m_multisample.alphaToCoverageEnable = VK_FALSE;
  m_multisample.alphaToOneEnable = VK_FALSE;
}

void VMultisampleState::FillPipelineState(VkPipelineMultisampleStateCreateInfo &info)
{
  info = m_multisample;
}

VDepthStencilState::VDepthStencilState()
{
  m_depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_depthStencil.pNext = nullptr;
  m_depthStencil.flags = 0;
  m_depthStencil.depthTestEnable = VK_TRUE;
  m_depthStencil.depthWriteEnable = VK_TRUE;
  m_depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  m_depthStencil.depthBoundsTestEnable = VK_FALSE;
  m_depthStencil.stencilTestEnable = VK_FALSE;
  m_depthStencil.front = {};
  m_depthStencil.back = {};
  m_depthStencil.minDepthBounds = 0.0f;
  m_depthStencil.maxDepthBounds = 1.0f;
}

void VDepthStencilState::FillPipelineState(VkPipelineDepthStencilStateCreateInfo &info)
{
  info = m_depthStencil;
}

VBlendState::VBlendState(Blend blend)
{
  SetBlend(blend);
}

void VBlendState::SetBlend(Blend blend)
{
  m_blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  switch (blend) {
    case DISABLED:
      m_blendState.blendEnable = VK_FALSE;
      m_blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
      m_blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
      m_blendState.colorBlendOp = VK_BLEND_OP_ADD;
      m_blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      m_blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      m_blendState.alphaBlendOp = VK_BLEND_OP_ADD;
      break;
    case SRC_ALPHA:
      m_blendState.blendEnable = VK_TRUE;
      m_blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      m_blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      m_blendState.colorBlendOp = VK_BLEND_OP_ADD;
      m_blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      m_blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      m_blendState.alphaBlendOp = VK_BLEND_OP_ADD;
      break;
    default:
      assert(0);
  }
}

void VBlendState::FillPipelineState(VkPipelineColorBlendStateCreateInfo &info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.logicOpEnable = VK_FALSE;
  info.logicOp = VK_LOGIC_OP_COPY;
  info.attachmentCount = 1;
  info.pAttachments = &m_blendState;
  info.blendConstants[0] = m_Rconst;
  info.blendConstants[1] = m_Gconst;
  info.blendConstants[2] = m_Bconst;
  info.blendConstants[3] = m_Aconst;
}

VDynamicState::VDynamicState() :
  m_states{ VK_DYNAMIC_STATE_VIEWPORT }
{
}

void VDynamicState::FillPipelineState(VkPipelineDynamicStateCreateInfo &info)
{
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  info.pNext = nullptr;
  info.flags = 0;
  info.dynamicStateCount = (uint32_t)m_states.size();
  info.pDynamicStates = m_states.data();
}

VRenderPass::VRenderPass(VDevice &device) :
  m_device(&device)
{
  std::array<VkAttachmentDescription, 2> attachments;
  VkAttachmentDescription &color = attachments[0], &depth = attachments[1];

  color.flags = 0;
  color.format = m_device->m_swapchain->m_images[0]->m_format;
  color.samples = VK_SAMPLE_COUNT_1_BIT;
  color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorRef;
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  depth.flags = 0;
  depth.format = m_device->m_depth->m_format;
  depth.samples = VK_SAMPLE_COUNT_1_BIT;
  depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthRef;
  depthRef.attachment = 1;
  depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subPass;
  subPass.flags = 0;
  subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subPass.inputAttachmentCount = 0;
  subPass.pInputAttachments = nullptr;
  subPass.colorAttachmentCount = 1;
  subPass.pColorAttachments = &colorRef;
  subPass.pResolveAttachments = nullptr;
  subPass.pDepthStencilAttachment = &depthRef;
  subPass.preserveAttachmentCount = 0;
  subPass.pPreserveAttachments = nullptr;

  VkSubpassDependency subpassDep;
  subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpassDep.dstSubpass = 0;
  subpassDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDep.srcAccessMask = 0;
  subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpassDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo pass;
  pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  pass.pNext = nullptr;
  pass.flags = 0;
  pass.attachmentCount = (uint32_t)attachments.size();
  pass.pAttachments = attachments.data();
  pass.subpassCount = 1;
  pass.pSubpasses = &subPass;
  pass.dependencyCount = 1;
  pass.pDependencies = &subpassDep;

  VkResult err = vkCreateRenderPass(m_device->m_device, &pass, nullptr, &m_renderPass);
  if (err < 0)
    throw VGraphicsException("VRenderPass failed in vkCreateRenderPass", err);
}

VRenderPass::~VRenderPass()
{
  vkDestroyRenderPass(m_device->m_device, m_renderPass, nullptr);
}

VDescriptorPool::VDescriptorPool(VDevice &device, bool synchronize, uint32_t uniformDescriptors) :
  m_device(&device),
  m_lock(synchronize)
{
  VkDescriptorPoolSize poolSize;
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = uniformDescriptors;

  VkDescriptorPoolCreateInfo poolInfo;
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.pNext = nullptr;
  poolInfo.flags = 0;
  poolInfo.maxSets = 1;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;

  VkResult err = vkCreateDescriptorPool(m_device->m_device, &poolInfo, nullptr, &m_pool);
  if (err < 0)
    throw VGraphicsException("VDescriptorPool failed in vkCreateDescriptorPool", err);
}

VDescriptorPool::~VDescriptorPool()
{
  vkDestroyDescriptorPool(m_device->m_device, m_pool, nullptr);
}

VDescriptorSet *VDescriptorPool::CreateSet(VDescriptorSetLayout *layout)
{
  std::lock_guard<ConditionalLock> lock(m_lock);

  VkDescriptorSetAllocateInfo setInfo;
  setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setInfo.pNext = nullptr;
  setInfo.descriptorPool = m_pool;
  setInfo.descriptorSetCount = 1;
  setInfo.pSetLayouts = &layout->m_layout;

  VkDescriptorSet vkSet;
  VkResult err = vkAllocateDescriptorSets(m_device->m_device, &setInfo, &vkSet);
  if (err < 0)
    throw VGraphicsException("VDescriptorPool::CreateSet failed in vkAllocateDescriptorSets", err);

  return new VDescriptorSet(*this, vkSet);
}

void VDescriptorPool::CreateSets(std::vector<std::shared_ptr<VDescriptorSetLayout>> const &layouts, std::vector<std::unique_ptr<VDescriptorSet>> &sets)
{
  std::lock_guard<ConditionalLock> lock(m_lock);

  std::vector<VkDescriptorSetLayout> vkLayouts(layouts.size());
  for (int i = 0; i < layouts.size(); ++i)
    vkLayouts[i] = layouts[i]->m_layout;

  VkDescriptorSetAllocateInfo setInfo;
  setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setInfo.pNext = nullptr;
  setInfo.descriptorPool = m_pool;
  setInfo.descriptorSetCount = (uint32_t)vkLayouts.size();
  setInfo.pSetLayouts = vkLayouts.data();

  std::vector<VkDescriptorSet> vkSets(vkLayouts.size());
  VkResult err = vkAllocateDescriptorSets(m_device->m_device, &setInfo, vkSets.data());
  if (err < 0)
    throw VGraphicsException("VDescriptorPool::CreateSets failed in vkAllocateDescriptorSets", err);

  for (auto s : vkSets)
    sets.push_back(std::make_unique<VDescriptorSet>(*this, s));
}

VDescriptorSet::VDescriptorSet(VDescriptorPool &pool, VkDescriptorSet set) :
  m_pool(&pool), m_set(set)
{
}

VDescriptorSet::VDescriptorSet()
{
  vkFreeDescriptorSets(m_pool->m_device->m_device, m_pool->m_pool, 1, &m_set);
}

VPipelineCache::VPipelineCache(VDevice &device, bool synchronize, std::vector<uint8_t> const *initialData) :
  m_device(&device),
  m_lock(synchronize)
{
  if (initialData && !m_device->m_graphics->IsPipelineCacheDataCompatible(*initialData))
    initialData = nullptr;

  VkPipelineCacheCreateInfo cacheInfo;
  cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  cacheInfo.pNext = nullptr;
  cacheInfo.flags = 0;
  cacheInfo.initialDataSize = initialData ? (uint8_t)initialData->size() : 0;
  cacheInfo.pInitialData = initialData ? initialData->data() : nullptr;

  VkResult err = vkCreatePipelineCache(m_device->m_device, &cacheInfo, nullptr, &m_cache);
  if (err < 0)
    throw VGraphicsException("VPipelineCache failed in vkCreatePipelineCache", err);
}

VPipelineCache::~VPipelineCache()
{
  vkDestroyPipelineCache(m_device->m_device, m_cache, nullptr);
}

void VPipelineCache::GetData(std::vector<uint8_t> &data)
{
  std::lock_guard<ConditionalLock> lock(m_lock);
  size_t dataSize = 0;
  VkResult err = vkGetPipelineCacheData(m_device->m_device, m_cache, &dataSize, nullptr);
  if (err < 0)
    throw VGraphicsException("VPipelineCache::GetData failed in vkGetPipelineCacheData", err);
  data.resize(dataSize);
  err = vkGetPipelineCacheData(m_device->m_device, m_cache, &dataSize, data.data());
  if (err < 0)
    throw VGraphicsException("VPipelineCache::GetData failed in vkGetPipelineCacheData", err);
}

VGraphicsPipeline::VGraphicsPipeline(VPipelineCache &cache, VkPipeline pipeline) :
  m_cache(&cache), m_pipeline(pipeline)
{
}

VGraphicsPipeline::~VGraphicsPipeline()
{
  vkDestroyPipeline(m_cache->m_device->m_device, m_pipeline, nullptr);
}

VModelInstance::VModelInstance(std::shared_ptr<VModel> model) :
  m_model(model)
{
  InitDescriptorSets();
  InitCmdBuffer();
}

void VModelInstance::InitDescriptorSets()
{
  VDevice &device = GetDevice();
  auto &setLayouts = m_model->m_material->m_pipelineLayout->m_setLayouts;
  device.m_descriptorPool->CreateSets(setLayouts, m_descriptorSets);

  std::vector<VkDescriptorBufferInfo> bufferInfos;
  std::vector<VkWriteDescriptorSet> uniformWrites;
  // reserve the vectors so they don't get reallocated, we'll be getting pointers to elements
  bufferInfos.reserve(setLayouts.size());
  uniformWrites.reserve(setLayouts.size());
  m_uniformBuffers.resize(setLayouts.size());
  for (int i = 0; i < setLayouts.size(); ++i) {
    if (setLayouts[i]->HasUniformBuffer()) {
      assert(m_model->m_material->m_uniformContent[i].size() > 0);
      m_uniformBuffers[i] = std::make_unique<VBuffer>(device, true, m_model->m_material->m_uniformContent[i].size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
      void *content = m_uniformBuffers[i]->Map();
      memcpy(content, m_model->m_material->m_uniformContent[i].data(), m_model->m_material->m_uniformContent[i].size());
      m_uniformBuffers[i]->Unmap();

      VkDescriptorBufferInfo bufInfo;
      bufInfo.buffer = m_uniformBuffers[i]->m_buffer;
      bufInfo.offset = 0;
      bufInfo.range = m_uniformBuffers[i]->m_size;
      bufferInfos.push_back(bufInfo);

      VkWriteDescriptorSet write;
      write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write.pNext = nullptr;
      write.dstSet = m_descriptorSets[i]->m_set;
      write.dstBinding = 0;
      write.dstArrayElement = 0;
      write.descriptorCount = 1;
      write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write.pImageInfo = nullptr;
      write.pBufferInfo = &bufferInfos.back();
      write.pTexelBufferView = nullptr;
      uniformWrites.push_back(write);
    }
  }

  vkUpdateDescriptorSets(device.m_device, (uint32_t)uniformWrites.size(), uniformWrites.data(), 0, nullptr);
}

void VModelInstance::InitCmdBuffer()
{
  VDevice &device = GetDevice();
  m_cmdBuffer.reset(device.m_cmdPool->CreateBuffer(false));
  m_cmdBuffer->Begin(true, device.m_renderPass.get(), 0);

  m_model->m_material->SetDynamicState(m_cmdBuffer.get());

  m_cmdBuffer->BindPipeline(*m_model->m_pipeline);
  m_cmdBuffer->BindVertexBuffers(m_model->m_geometry->m_vertexBuffers);
  m_cmdBuffer->BindIndexBuffer(*m_model->m_geometry->m_indexBuffer);
  m_cmdBuffer->BindDescriptorSets(*m_model->m_material->m_pipelineLayout, m_descriptorSets);
  m_cmdBuffer->DrawIndexed(m_model->m_geometry->m_indexBuffer->GetIndexCount());

  m_cmdBuffer->End();
}
