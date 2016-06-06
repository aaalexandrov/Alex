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
class VDevice;
class VCmdBuffer;

class VSemaphore {
public:
  VDevice *m_device;
  VkSemaphore m_semaphore = VK_NULL_HANDLE;
  VkPipelineStageFlags m_stages;

  VSemaphore(VDevice &device, VkPipelineStageFlags stages);
  ~VSemaphore();
};

class VFence {
public:
  VDevice *m_device;
  VkFence m_fence = VK_NULL_HANDLE;

  VFence(VDevice &device, bool signaled = false);
  ~VFence();

  bool IsSignaled();
  void Reset();
  bool Wait(uint64_t nsTimeout = UINT64_MAX);
};

class VQueue {
public:
  VkQueue m_queue;
  VDevice *m_device;
  ConditionalLock m_lock;

  VQueue(VDevice &device, bool synchronize, VkQueue queue);
  ~VQueue();

  bool Submit(VCmdBuffer &cmdBuffer, std::vector<VSemaphore*> *waitSemaphores = nullptr);
};

class VImage {
public:
  VDevice *m_device;
  VkImage m_image = VK_NULL_HANDLE;
  VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageView m_view = VK_NULL_HANDLE;
  bool m_ownImage = true;

  VImage(VDevice &device);
  ~VImage();

  bool Init(VkImage image, VkFormat format, bool ownImage = true);
  void Done();
};

class VSwapchain {
public:
  VDevice *m_device;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  std::vector<VImage *> m_images;

  VSwapchain(VDevice &device);
  ~VSwapchain();

  bool Init();
  void Done();

  bool InitSwapchain();
  void DoneSwapchain();

  bool InitImages();
  void DoneImages();
};

class VCmdPool {
public:
  VkCommandPool m_pool;
  VDevice *m_device;
  ConditionalLock m_lock;

  VCmdPool(VDevice &device, bool synchronize, bool transient, bool resetBuffers);
  ~VCmdPool();

  void InitBufferInfo(VkCommandBufferAllocateInfo &bufInfo, bool primary, uint32_t count);

  VCmdBuffer *CreateBuffer(bool primary);
  void CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers);
};

class VCmdBuffer {
public:
  VCmdPool *m_pool;
  VkCommandBuffer m_buffer;
  VSemaphore *m_semaphore = nullptr;
  VFence *m_fence = nullptr;

  VCmdBuffer(VCmdPool &pool, VkCommandBuffer buffer);
  ~VCmdBuffer();

  void SetUseSemaphore(bool use, VkPipelineStageFlags stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  void SetUseFence(bool use);

  bool Begin(bool simultaneous = false);
  bool End();

  bool SetImageLayout(VImage &image, VkImageLayout layout, VkAccessFlags priorAccess, VkAccessFlags followingAccess, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
};

class VDevice {
public:
  VGraphics *m_graphics;
  std::vector<std::string> m_layerNames, m_extensionNames;
  VkDevice m_device = VK_NULL_HANDLE;
  VSwapchain *m_swapchain = nullptr;
  VQueue *m_queue = nullptr;
  VCmdPool *m_cmdPool = VK_NULL_HANDLE;
  VCmdBuffer* m_cmdBuffer = nullptr;

  PFN_vkCreateSwapchainKHR    m_CreateSwapchainKHR = nullptr;
  PFN_vkDestroySwapchainKHR   m_DestroySwapchainKHR = nullptr;
  PFN_vkGetSwapchainImagesKHR m_GetSwapchainImagesKHR = nullptr;
  PFN_vkAcquireNextImageKHR   m_AcquireNextImageKHR = nullptr;
  PFN_vkQueuePresentKHR       m_QueuePresentKHR = nullptr;

  VDevice(VGraphics &graphics);
  virtual ~VDevice();

  virtual bool Init();
  virtual void Done();

  virtual bool InitCapabilities();
  virtual void DoneCapabilities();

  virtual bool InitDevice();
  virtual void DoneDevice();

  virtual bool InitSwapchain();
  virtual void DoneSwapchain();

  virtual bool InitCmdBuffers(bool synchronize, uint32_t count);
  virtual void DoneCmdBuffers();

  virtual bool SubmitInitCommands();
};

class VGraphics
{
public:
  bool m_validate;
  VkInstance m_instance = VK_NULL_HANDLE;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties m_physicalDeviceProps;
  VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProps;
  std::vector<VkQueueFamilyProperties> m_queueFamilies;
  unsigned m_graphicsQueueFamily;
  VkSurfaceKHR m_surface;
  VkSurfaceFormatKHR m_surfaceFormat;
  VDevice *m_device = nullptr;

  std::vector<std::string> m_layerNames, m_extensionNames;
  VkDebugReportCallbackEXT m_debugReportCallback = nullptr;

  PFN_vkGetPhysicalDeviceSurfaceSupportKHR      m_GetPhysicalDeviceSurfaceSupportKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR m_GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      m_GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR m_GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
  PFN_vkDestroySurfaceKHR                       m_DestroySurfaceKHR = nullptr;

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

  virtual bool InitSurface();
  virtual void DoneSurface();

  virtual bool InitDevice();
  virtual void DoneDevice();

  static bool AddLayerName(std::vector<VkLayerProperties> const &layers, std::string const &name, std::vector<std::string> &layerNames);
  static bool AddExtensionName(std::vector<VkExtensionProperties> const &extensions, std::string const &name, std::vector<std::string> &extNames);
  static bool AppendExtensionProperties(std::vector<VkExtensionProperties> &extensions, std::string const &layerName);

  static std::vector<char const*> GetPtrArray(std::vector<std::string> const &strs);

  static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);
};

