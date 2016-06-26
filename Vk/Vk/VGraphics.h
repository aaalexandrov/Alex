#pragma once

#if defined(_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(linux)
  #define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vulkan/vulkan.h"
#include <mutex>

template <class T>
void DoneResource(T *&resource)
{
  if (resource) {
    resource->Done();
    delete resource;
    resource = nullptr;
  }
}

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

class VMemory {
public:
  VkDeviceMemory m_memory = VK_NULL_HANDLE;
  ConditionalLock m_lock;
  uint64_t m_size = 0;

  VMemory(bool synchronize);
  ~VMemory();

  bool Init(VDevice &device, uint64_t size, uint32_t validMemoryTypes, VkMemoryPropertyFlags memFlags = 0);
  void Done(VDevice &device);

  void *Map(VDevice &device, uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE);
  void Unmap(VDevice &device);
};

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
  VkImageCreateInfo m_imgInfo;
  VkImage m_image = VK_NULL_HANDLE;
  VMemory m_memory;
  VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkFormat m_format = VK_FORMAT_UNDEFINED;
  VkExtent3D m_size = { 0, 0, 0 };
  VkImageView m_view = VK_NULL_HANDLE;
  bool m_ownImage = true;

  VImage(VDevice &device);
  ~VImage();

  bool Init(VkFormat format, uint32_t width, uint32_t height, uint32_t depth = 1, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, bool linear = false);
  
  bool Init(VkImage image, VkFormat format, bool ownImage = true, VkImageViewType imgType = VK_IMAGE_VIEW_TYPE_2D);
  void Done();

  void *Map();
  void Unmap();

  static VkImageUsageFlags UsageFromFormat(VkFormat format);
  static VkImageAspectFlags AspectFromFormat(VkFormat format);
  static VkFormat FormatFromComponents(unsigned components);
  static VkImageViewType ViewTypeFromDimensions(uint32_t width, uint32_t height, uint32_t depth, uint32_t arrayLayers);
};

class VSampler {
public:
  VDevice *m_device;
  VkSampler m_sampler = VK_NULL_HANDLE;

  VSampler(VDevice &device);
  ~VSampler();

  bool Init(VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VkFilter magFilter = VK_FILTER_LINEAR, VkFilter minFilter = VK_FILTER_LINEAR, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, float maxAnisotropy = 16.0f);
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
  bool m_autoBegin = true;
  bool m_simultaneousBegin = false;
  bool m_begun = false;

  VCmdBuffer(VCmdPool &pool, VkCommandBuffer buffer);
  ~VCmdBuffer();

  void SetUseSemaphore(bool use, VkPipelineStageFlags stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  void SetUseFence(bool use);

  bool AutoBegin();
  bool AutoEnd();

  bool Begin(bool simultaneous = false);
  bool End();

  bool Reset(bool releaseResources = false);

  bool SetImageLayout(VImage &image, VkImageLayout layout, VkAccessFlags priorAccess, VkAccessFlags followingAccess);
  bool CopyImage(VImage &src, VImage &dst);
};

class VDevice {
public:
  VGraphics *m_graphics;
  std::vector<std::string> m_layerNames, m_extensionNames;
  VkDevice m_device = VK_NULL_HANDLE;
  VSwapchain *m_swapchain = nullptr;
  VImage *m_depth = nullptr;
  VQueue *m_queue = nullptr;
  VCmdPool *m_cmdPool = VK_NULL_HANDLE;
  VCmdBuffer *m_cmdBuffer = nullptr;
  VImage *m_staging = nullptr;

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

  virtual bool InitDepth();
  virtual void DoneDepth();

  virtual bool InitCmdBuffers(bool synchronize, uint32_t count);
  virtual void DoneCmdBuffers();

  virtual bool SubmitInitCommands();

  virtual VImage *LoadVImage(std::string const &filename);

  virtual bool UpdateStagingImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t components);
  virtual void FreeStagingImage();
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

  virtual uint32_t GetMemoryTypeIndex(uint32_t validTypeMask, VkMemoryPropertyFlags flags);

  static bool AddLayerName(std::vector<VkLayerProperties> const &layers, std::string const &name, std::vector<std::string> &layerNames);
  static bool AddExtensionName(std::vector<VkExtensionProperties> const &extensions, std::string const &name, std::vector<std::string> &extNames);
  static bool AppendExtensionProperties(std::vector<VkExtensionProperties> &extensions, std::string const &layerName);

  static std::vector<char const*> GetPtrArray(std::vector<std::string> const &strs);

  static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);
};

