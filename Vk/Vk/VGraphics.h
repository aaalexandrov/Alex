#pragma once

#if defined(_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(linux)
  #define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vulkan/vulkan.h"
#include <mutex>

#include "UniqueResource.h"

struct VGraphicsException : public std::runtime_error {
  VkResult m_err;
  explicit VGraphicsException(const char *msg, VkResult err) : std::runtime_error(msg), m_err(err) {}
};

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
  VDevice *m_device;
  VkDeviceMemory m_memory = VK_NULL_HANDLE;
  ConditionalLock m_lock;
  uint64_t m_size = 0;

  VMemory(VDevice &device, bool synchronize, uint64_t size, uint32_t validMemoryTypes, VkMemoryPropertyFlags memFlags = 0);
  ~VMemory();

  void *Map(uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE);
  void Unmap();
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
  VDevice *m_device;
  VkQueue m_queue;
  ConditionalLock m_lock;

  VQueue(VDevice &device, bool synchronize, VkQueue queue);
  ~VQueue();

  void Submit(VCmdBuffer &cmdBuffer, std::vector<VSemaphore*> *waitSemaphores = nullptr);
  void WaitIdle();
};

class VImage {
public:
  VDevice *m_device;
  VkImage m_image = VK_NULL_HANDLE;
  VkImageView m_view = VK_NULL_HANDLE;
  std::shared_ptr<VMemory> m_memory;
  VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkFormat m_format = VK_FORMAT_UNDEFINED;
  VkExtent3D m_size = { 0, 0, 0 };

  VImage(VDevice &device, bool synchronize, VkFormat format, uint32_t width, uint32_t height, uint32_t depth = 1, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, bool linear = false);
  VImage(VDevice &device, VkImage image, VkFormat format, VkImageViewType imgType = VK_IMAGE_VIEW_TYPE_2D);
  ~VImage();

  void *Map();
  void Unmap();

  static VkImageUsageFlags UsageFromFormat(VkFormat format);
  static VkImageAspectFlags AspectFromFormat(VkFormat format);
  static VkFormat FormatFromComponents(unsigned components);
  static VkImageViewType ViewTypeFromDimensions(uint32_t width, uint32_t height, uint32_t depth, uint32_t arrayLayers);

public:
  void Init(VkImage image, VkFormat format, VkImageViewType imgType);
};

class VSampler {
public:
  VDevice *m_device;
  VkSampler m_sampler = VK_NULL_HANDLE;

  VSampler(VDevice &device, VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VkFilter magFilter = VK_FILTER_LINEAR, VkFilter minFilter = VK_FILTER_LINEAR, VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, float maxAnisotropy = 16.0f);
  ~VSampler();
};

class VBuffer {
public:
  VDevice *m_device;
  VkBuffer m_buffer = VK_NULL_HANDLE;
  std::shared_ptr<VMemory> m_memory;
  VkBufferUsageFlags m_usage = 0;
  uint64_t m_size = 0;

  VBuffer(VDevice &device, bool synchronize, uint64_t size, VkBufferUsageFlags usage, bool hostVisible);
  ~VBuffer();

  void *Map();
  void Unmap();
};

class VShader {
public:
  VDevice        *m_device;
  VkShaderModule  m_shader = VK_NULL_HANDLE;

  VShader(VDevice &device, size_t size, uint32_t *code);
  ~VShader();
};

class VSwapchain {
public:
  VDevice *m_device;
  UniqueResource<VkSwapchainKHR> m_swapchain;
  std::vector<std::unique_ptr<VImage>> m_images;

  VSwapchain(VDevice &device);
  ~VSwapchain();

public:
  void InitSwapchain();
  void InitImages();
};

class VCmdPool {
public:
  VkCommandPool m_pool;
  VDevice *m_device;
  ConditionalLock m_lock;

  VCmdPool(VDevice &device, bool synchronize, bool transient, bool resetBuffers);
  ~VCmdPool();

  VCmdBuffer *CreateBuffer(bool primary);
  void CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers);

public:
  void InitBufferInfo(VkCommandBufferAllocateInfo &bufInfo, bool primary, uint32_t count);
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

  void AutoBegin();
  void AutoEnd();

  void Begin(bool simultaneous = false);
  void End();

  void Reset(bool releaseResources = false);

  void SetImageLayout(VImage &image, VkImageLayout layout, VkAccessFlags priorAccess, VkAccessFlags followingAccess);
  void CopyImage(VImage &src, VImage &dst);
  void CopyBuffer(VBuffer &src, VBuffer &dst, uint64_t srcOffset = 0, uint64_t dstOffset = 0, uint64_t size = VK_WHOLE_SIZE);
};

class VDevice {
public:
  VGraphics *m_graphics;
  std::vector<std::string> m_layerNames, m_extensionNames;
  UniqueResource<VkDevice> m_device;
  std::unique_ptr<VSwapchain> m_swapchain;
  std::unique_ptr<VImage> m_depth;
  std::unique_ptr<VQueue> m_queue;
  std::unique_ptr<VCmdPool> m_cmdPool;
  std::unique_ptr<VCmdBuffer> m_cmdBuffer;
  std::unique_ptr<VImage> m_staging;

  PFN_vkCreateSwapchainKHR    m_CreateSwapchainKHR = nullptr;
  PFN_vkDestroySwapchainKHR   m_DestroySwapchainKHR = nullptr;
  PFN_vkGetSwapchainImagesKHR m_GetSwapchainImagesKHR = nullptr;
  PFN_vkAcquireNextImageKHR   m_AcquireNextImageKHR = nullptr;
  PFN_vkQueuePresentKHR       m_QueuePresentKHR = nullptr;

  VDevice(VGraphics &graphics);
  ~VDevice();

  VImage *LoadVImage(std::string const &filename);
  VBuffer *LoadVBuffer(uint64_t size, VkBufferUsageFlags usage, void *data);
  VShader *LoadVShader(std::string const &filename);

  void UpdateStagingImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t components);
  void FreeStagingImage();

public:
  void InitCapabilities();
  void InitDevice();
  void InitDepth();
  void InitCmdBuffers(bool synchronize, uint32_t count);

  void SubmitInitCommands();
};

class VGraphics
{
public:
  bool m_validate;
  UniqueResource<VkInstance> m_instance;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties m_physicalDeviceProps;
  VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProps;
  std::vector<VkQueueFamilyProperties> m_queueFamilies;
  unsigned m_graphicsQueueFamily;
  UniqueResource<VkSurfaceKHR> m_surface;
  VkSurfaceFormatKHR m_surfaceFormat;
  std::unique_ptr<VDevice> m_device;

  std::vector<std::string> m_layerNames, m_extensionNames;
  UniqueResource<VkDebugReportCallbackEXT> m_debugReportCallback;

  PFN_vkGetPhysicalDeviceSurfaceSupportKHR      m_GetPhysicalDeviceSurfaceSupportKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR m_GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      m_GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR m_GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
  PFN_vkDestroySurfaceKHR                       m_DestroySurfaceKHR = nullptr;

  PFN_vkCreateDebugReportCallbackEXT  m_CreateDebugReportCallbackEXT = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT m_DestroyDebugReportCallbackEXT = nullptr;
  PFN_vkDebugReportMessageEXT         m_DebugReportMessageEXT = nullptr;

  VGraphics(bool validate, std::string const &appName, uint32_t appVersion, uintptr_t instanceID, uintptr_t windowID);
  ~VGraphics();

public:
  void InitInstance(std::string const &appName, uint32_t appVersion);
  void InitPhysicalDevice();
  void InitSurface(uintptr_t instanceID, uintptr_t windowID);

  uint32_t GetMemoryTypeIndex(uint32_t validTypeMask, VkMemoryPropertyFlags flags);

  static bool AddLayerName(std::vector<VkLayerProperties> const &layers, std::string const &name, std::vector<std::string> &layerNames);
  static bool AddExtensionName(std::vector<VkExtensionProperties> const &extensions, std::string const &name, std::vector<std::string> &extNames);
  static bool AppendExtensionProperties(std::vector<VkExtensionProperties> &extensions, std::string const &layerName);

  static std::vector<char const*> GetPtrArray(std::vector<std::string> const &strs);

  static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);
};

