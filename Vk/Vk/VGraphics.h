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

  void Submit(VCmdBuffer &cmdBuffer, std::vector<VSemaphore*> *waitSemaphores = nullptr);
  void WaitIdle();
};

class VImage {
public:
  VDevice *m_device;
  UniqueResource<VkImage> m_image;
  UniqueResource<VkImageView> m_view;
  std::shared_ptr<VMemory> m_memory;
  VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkFormat m_format = VK_FORMAT_UNDEFINED;
  VkExtent3D m_size = { 0, 0, 0 };

  VImage(VDevice &device, bool synchronize, VkFormat format, uint32_t width, uint32_t height, uint32_t depth = 1, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, bool linear = false);
  VImage(VDevice &device, VkImage image, VkFormat format, VkImageViewType imgType = VK_IMAGE_VIEW_TYPE_2D);

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
  UniqueResource<VkBuffer> m_buffer;
  std::shared_ptr<VMemory> m_memory;
  VkBufferUsageFlags m_usage = 0;
  uint64_t m_size = 0;

  VBuffer(VDevice &device, bool synchronize, uint64_t size, VkBufferUsageFlags usage, bool hostVisible);

  void *Map();
  void Unmap();
};

class VShader {
public:
  VDevice               *m_device;
  VkShaderModule         m_shader = VK_NULL_HANDLE;
  VkShaderStageFlagBits  m_stageBits;

  VShader(VDevice &device, VkShaderStageFlagBits stageBits, size_t size, uint32_t *code);
  ~VShader();

  void FillPipelineInfo(VkPipelineShaderStageCreateInfo &info);
};

class VSwapchain {
public:
  VDevice *m_device;
  UniqueResource<VkSwapchainKHR> m_swapchain;
  std::vector<std::unique_ptr<VImage>> m_images;

  VSwapchain(VDevice &device);

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

class VDescriptorSetLayout {
public:
  VDevice *m_device;
  VkDescriptorSetLayout m_layout;

  VDescriptorSetLayout(VDevice &device);
  ~VDescriptorSetLayout();
};

class VPipelineLayout {
public:
  VDevice *m_device;
  VkPipelineLayout m_layout;
  std::vector<std::shared_ptr<VDescriptorSetLayout>> m_setLayouts;

  VPipelineLayout(VDevice &m_device, std::vector<std::shared_ptr<VDescriptorSetLayout>> const &setLayouts);
  ~VPipelineLayout();
};

struct VVertexInfo {
  std::vector<VkVertexInputAttributeDescription> m_attr;
  VkVertexInputBindingDescription m_binding;
  uint32_t m_stride = 0;

  void AddAttribute(VkFormat format, uint32_t offset);
  void FillPipelineInfo(VkPipelineVertexInputStateCreateInfo &info);
};

class VVertexBuffer {
public:
  std::shared_ptr<VVertexInfo> m_vertexInfo;
  std::unique_ptr<VBuffer> m_buffer;

  VVertexBuffer(VDevice &device, uint64_t size, void *data, std::shared_ptr<VVertexInfo> vertexInfo = std::make_shared<VVertexInfo>());
};

class VIndexBuffer {
public:
  VkIndexType m_type;
  std::unique_ptr<VBuffer> m_buffer;

  VIndexBuffer(VDevice &device, uint64_t count, uint16_t *indices);
  VIndexBuffer(VDevice &device, uint64_t count, uint32_t *indices);

  static uint32_t TypeSize(VkIndexType type);
};

class VViewportState {
public:
  VkViewport m_viewport;
  VkRect2D m_scissor;

  VViewportState(uint32_t width, uint32_t height);

  void SetSize(uint32_t width, uint32_t height);
  void FillPilelineInfo(VkPipelineViewportStateCreateInfo &info);
};

class VRasterizationState {
public:
  VkPipelineRasterizationStateCreateInfo m_rasterization;

  VRasterizationState();

  void FillPipelineInfo(VkPipelineRasterizationStateCreateInfo &info);
};

class VMultisampleState {
public:
  VkPipelineMultisampleStateCreateInfo m_multisample;

  VMultisampleState();

  void FillPipelineState(VkPipelineMultisampleStateCreateInfo &info);
};

class VDepthStencilState {
public:
  VkPipelineDepthStencilStateCreateInfo m_depthStencil;

  VDepthStencilState();

  void FillPipelineState(VkPipelineDepthStencilStateCreateInfo &info);
};

class VBlendState {
public:
  enum Blend {
    DISABLED,
    SRC_ALPHA,
  };

  VkPipelineColorBlendAttachmentState m_blendState;
  float m_Rconst = 0.0f, m_Gconst = 0.0f, m_Bconst = 0.0f, m_Aconst = 0.0f;

  VBlendState(Blend blend);

  void SetBlend(Blend blend);

  void FillPipelineState(VkPipelineColorBlendStateCreateInfo &info);
};

class VDynamicState {
public:
  std::vector<VkDynamicState> m_states;

  VDynamicState();

  void FillPipelineState(VkPipelineDynamicStateCreateInfo &info);
};

class VRenderPass {
public:
  VDevice *m_device;
  VkRenderPass m_renderPass = VK_NULL_HANDLE;

  VRenderPass(VDevice &device);
  ~VRenderPass();
};

class VDescriptorSet;
class VDescriptorPool {
public:
  VDevice *m_device;
  VkDescriptorPool m_pool;
  ConditionalLock m_lock;

  VDescriptorPool(VDevice &device, bool synchronize, uint32_t uniformDescriptors);
  ~VDescriptorPool();

  VDescriptorSet *CreateSet(VDescriptorSetLayout *layout);
  void CreateSets(std::vector<VDescriptorSetLayout*> const &layouts, std::vector<VDescriptorSet*> &sets);
};

class VDescriptorSet {
public:
  VDescriptorPool *m_pool;
  VkDescriptorSet m_set;

  VDescriptorSet(VDescriptorPool &pool, VkDescriptorSet set);
  VDescriptorSet();
};

class VGraphicsPipeline;
class VPipelineCache {
public:
  VDevice *m_device;
  VkPipelineCache m_cache;
  ConditionalLock m_lock;

  VPipelineCache(VDevice &device, bool synchronize = true, std::vector<uint8_t> const *initialData = nullptr);
  ~VPipelineCache();

  void GetData(std::vector<uint8_t> &data);
};

class VGraphicsPipeline {
public:
  VPipelineCache *m_cache;
  VkPipeline m_pipeline;

  VGraphicsPipeline(VPipelineCache &cache, VkPipeline pipeline);
  ~VGraphicsPipeline();
};

class VMaterial {
public:
  std::vector<std::shared_ptr<VShader>> m_shaders;
  std::shared_ptr<VBlendState> m_blendState;
  std::shared_ptr<VDepthStencilState> m_depthStencilState;
  std::shared_ptr<VRasterizationState> m_rasterizationState;
  std::shared_ptr<VMultisampleState> m_multisampleState;
  std::shared_ptr<VViewportState> m_viewportState;
  std::shared_ptr<VDynamicState> m_dynamicState;

  std::unique_ptr<VPipelineLayout> m_pipelineLayout;

  VMaterial(std::vector<std::shared_ptr<VShader>> const &shaders);

  VDevice &GetDevice() const { return *m_shaders[0]->m_device; }

  void CreatePipelineLayout();
};

class VGeometry {
public:
  std::shared_ptr<VVertexBuffer> m_vertexBuffer;
  std::shared_ptr<VIndexBuffer> m_indexBuffer;
  VkPrimitiveTopology m_topology;

  VGeometry(VkPrimitiveTopology topology, std::shared_ptr<VIndexBuffer> indexBuffer, std::shared_ptr<VVertexBuffer> vertexBuffer);

  void FillPipelineInfo(VkPipelineInputAssemblyStateCreateInfo &info);
};

class VModel {
public:
  std::shared_ptr<VGeometry> m_geometry;
  std::shared_ptr<VMaterial> m_material;
  std::unique_ptr<VGraphicsPipeline> m_pipeline;

  void CreatePipeline(uint32_t subpass = 0);
};

class VModelInstance {
public:
  std::shared_ptr<VModel> m_model;
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
  std::unique_ptr<VRenderPass> m_renderPass;
  std::unique_ptr<VPipelineCache> m_pipelineCache;
  std::unique_ptr<VDescriptorPool> m_descriptorPool;
  std::shared_ptr<VViewportState> m_viewportState;
  std::shared_ptr<VMultisampleState> m_multisampleState;
  std::shared_ptr<VDynamicState> m_dynamicState;
  std::unique_ptr<VImage> m_stagingImage;
  std::unique_ptr<VBuffer> m_stagingBuffer;

  PFN_vkCreateSwapchainKHR    m_CreateSwapchainKHR = nullptr;
  PFN_vkDestroySwapchainKHR   m_DestroySwapchainKHR = nullptr;
  PFN_vkGetSwapchainImagesKHR m_GetSwapchainImagesKHR = nullptr;
  PFN_vkAcquireNextImageKHR   m_AcquireNextImageKHR = nullptr;
  PFN_vkQueuePresentKHR       m_QueuePresentKHR = nullptr;

  VDevice(VGraphics &graphics);

  VImage *LoadVImage(std::string const &filename);
  VBuffer *LoadVBuffer(uint64_t size, VkBufferUsageFlags usage, void *data);
  VShader *LoadVShader(std::string const &filename);

  void UpdateStagingImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t components);
  void FreeStagingImage();

  void UpdateStagingBuffer(uint64_t size);
  void FreeStagingBuffer();

public:
  void InitCapabilities();
  void InitDevice();
  void InitDepth();
  void InitCmdBuffers(bool synchronize, uint32_t count);
  void InitViewportState();

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

  bool IsPipelineCacheDataCompatible(std::vector<uint8_t> const &data);

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

