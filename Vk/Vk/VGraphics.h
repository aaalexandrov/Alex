#pragma once

#if defined(_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(linux)
  #define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vulkan/vulkan.h"
#include <mutex>

class ConditionalLock {
public:
  std::recursive_mutex *m_mutex;

  ConditionalLock(bool createMutex) : m_mutex(createMutex ? new std::recursive_mutex() : nullptr) {}
  ~ConditionalLock() { if (m_mutex) delete m_mutex; }

  void lock() { if (m_mutex) m_mutex->lock(); }
  void unlock() { if (m_mutex) m_mutex->unlock(); }
  bool try_lock() { if (m_mutex) return m_mutex->try_lock(); return true; }
};

class VGraphics;
class VCmdBuffer;
class VCmdPool {
public:
  VkCommandPool m_pool;
  VGraphics *m_graphics;
  ConditionalLock m_lock;

  VCmdPool(VGraphics &graphics, bool synchronize, bool transient, bool resetBuffers);
  ~VCmdPool();

  void InitBufferInfo(VkCommandBufferAllocateInfo &bufInfo, bool primary, uint32_t count);

  VCmdBuffer *CreateBuffer(bool primary);
  void CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers);
};

class VCmdBuffer {
public:
  VCmdPool *m_pool;
  VkCommandBuffer m_buffer;

  VCmdBuffer(VCmdPool &pool, VkCommandBuffer buffer);
  ~VCmdBuffer();
};

class VQueue {
public:
  VkQueue m_queue;
  VGraphics *m_graphics;
  ConditionalLock m_lock;

  VQueue(VGraphics &graphics, bool synchronize, VkQueue queue);
  ~VQueue();
};

class VGraphics
{
public:
  bool m_validate;
  VkInstance m_instance = nullptr;
  VkPhysicalDevice m_physicalDevice = nullptr;
  VkPhysicalDeviceProperties m_physicalDeviceProps;
  VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProps;
  std::vector<VkQueueFamilyProperties> m_queueFamilies;
  unsigned m_graphicsQueueFamily;
  VkSurfaceKHR m_surface;
  VkSurfaceFormatKHR m_surfaceFormat;
  VkDevice m_device = nullptr;
  VQueue *m_queue = nullptr;
  VCmdPool *m_cmdPool = nullptr;
  std::vector<VCmdBuffer*> m_cmdBuffers;

  std::vector<std::string> m_instanceLayerNames, m_instanceExtensionNames, m_deviceLayerNames, m_deviceExtensionNames;
  VkDebugReportCallbackEXT m_debugReportCallback = nullptr;

  PFN_vkGetPhysicalDeviceSurfaceSupportKHR      m_GetPhysicalDeviceSurfaceSupportKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR m_GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      m_GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR m_GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
  PFN_vkDestroySurfaceKHR                       m_DestroySurfaceKHR = nullptr;

  PFN_vkCreateSwapchainKHR    m_CreateSwapchainKHR = nullptr;
  PFN_vkDestroySwapchainKHR   m_DestroySwapchainKHR = nullptr;
  PFN_vkGetSwapchainImagesKHR m_GetSwapchainImagesKHR = nullptr;
  PFN_vkAcquireNextImageKHR   m_AcquireNextImageKHR = nullptr;
  PFN_vkQueuePresentKHR       m_QueuePresentKHR = nullptr;

  PFN_vkCreateDebugReportCallbackEXT  m_CreateDebugReportCallbackEXT = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT m_DestroyDebugReportCallbackEXT = nullptr;
  PFN_vkDebugReportMessageEXT         m_DebugReportMessageEXT = nullptr;

  VGraphics(bool validate);
  virtual ~VGraphics();

  virtual bool Init();
  virtual void Done();

public:
  virtual bool InitInstance();
  virtual void DoneInstance();

  virtual bool InitPhysicalDevice();
  virtual void DonePhysicalDevice();

  virtual bool InitSwapchain();
  virtual void DoneSwapchain();

  virtual bool InitDevice();
  virtual void DoneDevice();

  virtual bool InitCmdBuffers(bool synchronize, uint32_t count);
  virtual void DoneCmdBuffers();

  bool AddLayerName(std::vector<VkLayerProperties> const &layers, std::string const &name, std::vector<std::string> &layerNames);
  bool AddExtensionName(std::vector<VkExtensionProperties> const &extensions, std::string const &name, std::vector<std::string> &extNames);
  bool AppendExtensionProperties(std::vector<VkExtensionProperties> &extensions, std::string const &layerName);

  static std::vector<char const*> GetPtrArray(std::vector<std::string> const &strs);

  static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);
};

