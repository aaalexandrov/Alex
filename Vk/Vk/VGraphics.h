#pragma once

#if defined(_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(linux)
  #define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vulkan/vulkan.h"
#include <mutex>
#include <queue>

#include "UniqueResource.h"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

class VTrackedResource {
public:
  uint64_t m_frameUpdated = 0;
  uint64_t m_frameUsed = 0;
};

template <class R, class P, class D>
class VResourcePool {
public:
  struct ResourceFrameUsedComparer {
    bool operator()(std::unique_ptr<R> const &r1, std::unique_ptr<R> const &r2) { return r1->m_frameUsed > r2->m_frameUsed; }
  };
  typedef std::priority_queue<std::unique_ptr<R>, std::vector<std::unique_ptr<R>>, ResourceFrameUsedComparer> ResourceQueue;

  std::unique_ptr<P> m_pool;
  ResourceQueue m_released;

  // To be implemented by derived classes
  //bool CompatibleResource(R *resource, Args... args);
  //R *CreateResource(Args... args);

  template <typename... Args>
  std::unique_ptr<R> GetResource(uint64_t frameMax, Args... args)
  {
    std::vector<std::unique_ptr<R>> inspected;
    std::unique_ptr<R> found;
    while (!m_released.empty() && m_released.top()->m_frameUsed <= frameMax)
    {
      found = std::move(const_cast<std::unique_ptr<R>&>(m_released.top()));
      m_released.pop();
      if (static_cast<D*>(this)->CompatibleResource(found.get(), std::forward<Args>(args)...))
        break;
      inspected.emplace_back(std::move(found));
    }
    for (auto &r : inspected)
      m_released.push(std::move(r));
    if (!found)
      found = std::unique_ptr<R>(static_cast<D*>(this)->CreateResource(std::forward<Args>(args)...));
    return found;
  }

  void ReleaseResource(std::unique_ptr<R> &&resource)
  {
    if (resource)
      m_released.push(std::move(resource));
  }

  void FreeReleased(uint64_t frameMax)
  {
    while (!m_released.empty() && m_released.top()->m_frameUsed <= frameMax)
      m_released.pop();
  }
};

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
  VImage(VDevice &device, VkImage image, VkFormat format, uint32_t width, uint32_t height, uint32_t depth, VkImageViewType imgType = VK_IMAGE_VIEW_TYPE_2D);

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

class VRenderPass;
class VFramebuffer {
public:
  VDevice *m_device;
  VkFramebuffer m_framebuffer;

  VFramebuffer(VRenderPass *renderPass, std::vector<VImage*> &attachments);
  ~VFramebuffer();
};

class VCmdPool {
public:
  VDevice *m_device;
  VkCommandPool m_pool;
  ConditionalLock m_lock;

  VCmdPool(VDevice &device, bool synchronize, bool transient, bool resetBuffers);
  ~VCmdPool();

  VCmdBuffer *CreateBuffer(bool primary);
  void CreateBuffers(bool primary, uint32_t count, std::vector<VCmdBuffer*> &buffers);

  void Reset(bool releasePoolResources);
public:
  void InitBufferInfo(VkCommandBufferAllocateInfo &bufInfo, bool primary, uint32_t count);
};

class VGraphicsPipeline;
class VPipelineLayout;
class VDescriptorSet;
class VVertexBuffer;
class VIndexBuffer;
class VCmdBuffer : public VTrackedResource {
public:
  VCmdPool *m_pool;
  VkCommandBuffer m_buffer;
  VSemaphore *m_semaphore = nullptr;
  VFence *m_fence = nullptr;
  bool m_autoBegin;
  bool m_simultaneousBegin = false;
  bool m_begun = false;
  bool m_primary;

  VCmdBuffer(VCmdPool &pool, VkCommandBuffer buffer, bool primary);
  ~VCmdBuffer();

  void SetUseSemaphore(bool use, VkPipelineStageFlags stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
  void SetUseFence(bool use, bool createSignaled = false);

  void AutoBegin();
  void AutoEnd();

  void Begin(bool simultaneous = false, VRenderPass *renderPass = nullptr, uint32_t subpass = 0);
  void End();

  void Reset(bool releaseResources = false);

  void SetImageLayout(VImage &image, VkImageLayout layout, VkAccessFlags priorAccess, VkAccessFlags followingAccess);
  void CopyImage(VImage &src, VImage &dst);
  void CopyBuffer(VBuffer &src, VBuffer &dst, uint64_t srcOffset = 0, uint64_t dstOffset = 0, uint64_t size = VK_WHOLE_SIZE);
  void SetViewport(VkViewport &viewport);
  void BindPipeline(VGraphicsPipeline &pipeline);
  void BindDescriptorSets(VPipelineLayout &pilelineLayout, std::vector<std::unique_ptr<VDescriptorSet>> const &sets, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
  void BindVertexBuffers(std::vector<std::shared_ptr<VVertexBuffer>> &vertexBuffers);
  void BindIndexBuffer(VIndexBuffer &indexBuffer, VkDeviceSize offset = 0);
  void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);
  void ExecuteCommands(VCmdBuffer &cmdBuffer);
  void ExecuteCommands(std::vector<VCmdBuffer*> const &cmdBuffers);
  void BeginRenderPass(VRenderPass &pass, VFramebuffer &framebuffer, uint32_t width, uint32_t height, std::vector<VkClearValue> const &clearValues, bool secondaryBuffers);
  void EndRenderPass();
  void NextSubpass(bool secondaryBuffers);
  void FramebufferBarrier(VImage &image, bool present2graphics);
};

class VPooledBuffers : public VResourcePool<VCmdBuffer, VCmdPool, VPooledBuffers> {
public:
  bool CompatibleResource(VCmdBuffer *cmdBuf, bool primary) const { return cmdBuf->m_primary == primary; }
  VCmdBuffer *CreateResource(bool primary) { return m_pool->CreateBuffer(primary); }
};

class VSwapchain {
public:
  struct Surface {
    std::unique_ptr<VImage> m_image;
    std::unique_ptr<VCmdBuffer> m_cmdRender;
    std::unique_ptr<VFramebuffer> m_framebuffer;
  };

  VDevice *m_device;
  UniqueResource<VkSwapchainKHR> m_swapchain;
  std::vector<Surface> m_surfaces;

  VSwapchain(VDevice &device, uint32_t width, uint32_t height);

  uint32_t GetWidth() const { return m_surfaces[0].m_image->m_size.width; }
  uint32_t GetHeight() const { return m_surfaces[0].m_image->m_size.height; }
  VkFormat GetFormat() const { return m_surfaces[0].m_image->m_format; }

  Surface &AcquireNext(VSemaphore *semaphore);
  void Present(Surface &surface);

public:
  void InitSwapchain(uint32_t width, uint32_t height);
  void InitSurfaces(uint32_t width, uint32_t height);
  void InitFramebuffers(VRenderPass &renderPass, VImage *depth);
};

class VDescriptorSetLayout {
public:
  VDevice *m_device;
  VkDescriptorSetLayout m_layout;

  VDescriptorSetLayout(VDevice &device);
  ~VDescriptorSetLayout();

  bool HasUniformBuffer() { return true; }
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
  VkVertexInputRate m_rate = VK_VERTEX_INPUT_RATE_VERTEX;
  uint32_t m_stride = 0;

  void AddAttribute(VkFormat format, uint32_t offset);
  void FillPipelineInfo(uint32_t binding, std::vector<VkVertexInputAttributeDescription> &attr, std::vector<VkVertexInputBindingDescription> &bindings);

  static void AddAttribute(uint32_t binding, std::vector<VkVertexInputAttributeDescription> &attr, VkFormat format, uint32_t offset);
};

class VVertexBuffer {
public:
  std::shared_ptr<VVertexInfo> m_vertexInfo;
  std::unique_ptr<VBuffer> m_buffer;

  VVertexBuffer(VDevice &device, uint64_t size, void *data, std::shared_ptr<VVertexInfo> vertexInfo = std::make_shared<VVertexInfo>());
};

class VIndexBuffer {
public:
  std::unique_ptr<VBuffer> m_buffer;
  VkIndexType m_type;

  VIndexBuffer(VDevice &device, uint64_t count, uint16_t *indices);
  VIndexBuffer(VDevice &device, uint64_t count, uint32_t *indices);

  uint32_t GetIndexCount() const { return (uint32_t)(m_buffer->m_size / TypeSize(m_type)); }

  static uint32_t TypeSize(VkIndexType type);
};

class VDeviceState {
public:
  VDevice *m_device;
  uint64_t m_frameUpdated;

  VDeviceState(VDevice &device);

  void StateUpdated();
};

class VViewportState : public VDeviceState {
public:
  VkViewport m_viewport;
  VkRect2D m_scissor;

  VViewportState(VDevice &device, uint32_t width, uint32_t height);

  void SetSize(uint32_t width, uint32_t height);
  void FillPilelineInfo(VkPipelineViewportStateCreateInfo &info);
};

class VRasterizationState : public VDeviceState {
public:
  VkPipelineRasterizationStateCreateInfo m_rasterization;

  VRasterizationState(VDevice &device);

  void FillPipelineInfo(VkPipelineRasterizationStateCreateInfo &info);
};

class VMultisampleState : public VDeviceState {
public:
  VkPipelineMultisampleStateCreateInfo m_multisample;

  VMultisampleState(VDevice &device);

  void FillPipelineState(VkPipelineMultisampleStateCreateInfo &info);
};

class VDepthStencilState : public VDeviceState {
public:
  VkPipelineDepthStencilStateCreateInfo m_depthStencil;

  VDepthStencilState(VDevice &device);

  void FillPipelineState(VkPipelineDepthStencilStateCreateInfo &info);
};

class VBlendState : public VDeviceState {
public:
  enum Blend {
    DISABLED,
    SRC_ALPHA,
  };

  VkPipelineColorBlendAttachmentState m_blendState;
  float m_Rconst = 0.0f, m_Gconst = 0.0f, m_Bconst = 0.0f, m_Aconst = 0.0f;

  VBlendState(VDevice &device, Blend blend);

  void SetBlend(Blend blend);

  void FillPipelineState(VkPipelineColorBlendStateCreateInfo &info);
};

class VDynamicState : public VDeviceState {
public:
  std::vector<VkDynamicState> m_states;

  VDynamicState(VDevice &device);

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

  VDescriptorPool(VDevice &device, bool synchronize, bool resetDescriptors, uint32_t uniformDescriptors);
  ~VDescriptorPool();

  VDescriptorSet *CreateSet(std::shared_ptr<VDescriptorSetLayout> const &layout);
  void CreateSets(std::vector<std::shared_ptr<VDescriptorSetLayout>> const &layouts, std::vector<std::unique_ptr<VDescriptorSet>> &sets);

  void Reset();
};

class VDescriptorSet : public VTrackedResource {
public:
  VDescriptorPool *m_pool;
  VkDescriptorSet m_set;
  std::shared_ptr<VDescriptorSetLayout> m_layout;

  VDescriptorSet(VDescriptorPool &pool, VkDescriptorSet set, std::shared_ptr<VDescriptorSetLayout> const &layout);
  ~VDescriptorSet();

  void Update(std::unique_ptr<VBuffer> const &uniformBuffer);
  static void Update(std::vector<std::unique_ptr<VDescriptorSet>> &sets, std::vector<std::unique_ptr<VBuffer>> const &uniformBuffers);
public:
  void FillPipelineInfo(std::unique_ptr<VBuffer> const &uniformBuffer, std::vector<VkDescriptorBufferInfo> &bufferInfos, std::vector<VkWriteDescriptorSet> &uniformWrites);
};

class VPooledDescriptors : VResourcePool<VDescriptorSet, VDescriptorPool, VPooledDescriptors> {
public:
  bool CompatibleResource(VDescriptorSet *descSet, std::shared_ptr<VDescriptorSetLayout> const &layout) const { return descSet->m_layout == layout; }
  VDescriptorSet *CreateResource(std::shared_ptr<VDescriptorSetLayout> const &layout) { return m_pool->CreateSet(layout); }
};

class VGraphicsPipeline;
class VModel;
class VPipelineCache {
public:
  VDevice *m_device;
  VkPipelineCache m_cache;
  ConditionalLock m_lock;

  VPipelineCache(VDevice &device, bool synchronize = true, std::vector<uint8_t> const *initialData = nullptr);
  ~VPipelineCache();

  void GetData(std::vector<uint8_t> &data);

  VGraphicsPipeline *CreatePipeline(VModel &model, uint32_t subpass);
};

class VGraphicsPipeline : public VTrackedResource {
public:
  VPipelineCache *m_cache;
  VkPipeline m_pipeline;

  VGraphicsPipeline(VPipelineCache &cache, VkPipeline pipeline);
  ~VGraphicsPipeline();
};

class VPooledPipelines : public VResourcePool<VGraphicsPipeline, VPipelineCache, VPooledPipelines> {
public:
  // We override GetResource to not use the released pipelines but always create a new one
  std::unique_ptr<VGraphicsPipeline> GetResource(uint64_t frameMax, VModel &model, uint32_t subpass)
  {
    return std::unique_ptr<VGraphicsPipeline>(m_pool->CreatePipeline(model, subpass));
  }
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
  std::vector<std::vector<uint8_t>> m_uniformContent;

  std::unique_ptr<VPipelineLayout> m_pipelineLayout;

  VMaterial(std::vector<std::shared_ptr<VShader>> const &shaders);

  VDevice &GetDevice() const { return *m_shaders[0]->m_device; }
  uint64_t GetFrameUpdated() const;

  void CreatePipelineLayout();
  void SetDynamicState(VCmdBuffer *cmdBuffer);
};

class VGeometry {
public:
  std::vector<std::shared_ptr<VVertexBuffer>> m_vertexBuffers;
  std::shared_ptr<VIndexBuffer> m_indexBuffer;
  VkPrimitiveTopology m_topology;

  VGeometry(VkPrimitiveTopology topology, std::shared_ptr<VIndexBuffer> indexBuffer, std::vector<std::shared_ptr<VVertexBuffer>> &vertexBuffers);

  VDevice &GetDevice() const { return *m_vertexBuffers[0]->m_buffer->m_device; }
public:
  void FillPipelineInfo(VkPipelineInputAssemblyStateCreateInfo &info);
};

class VModel {
public:
  std::shared_ptr<VGeometry> m_geometry;
  std::shared_ptr<VMaterial> m_material;
  std::unique_ptr<VGraphicsPipeline> m_pipeline;

  VModel(std::shared_ptr<VGeometry> geometry, std::shared_ptr<VMaterial> material);

  uint64_t GetFrameUpdated() const { return m_pipeline->m_frameUpdated; }

  void Update();
};

class VModelInstance {
public:
  std::shared_ptr<VModel> m_model;
  std::vector<std::unique_ptr<VDescriptorSet>> m_descriptorSets;
  std::vector<std::unique_ptr<VBuffer>> m_uniformBuffers;
  std::unique_ptr<VCmdBuffer> m_cmdBuffer;
  uint64_t m_frameModified = 0;

  VModelInstance(std::shared_ptr<VModel> model);

  VDevice &GetDevice() const { return m_model->m_material->GetDevice(); }

  void Update();
public:
  void InitDescriptorSets();
  void UpdateCmdBuffer();
};

class VDevice {
public:
  const uint32_t INVALID_DIM = std::numeric_limits<uint32_t>::max();

  VGraphics *m_graphics;
  std::vector<std::string> m_layerNames, m_extensionNames;
  UniqueResource<VkDevice> m_device;
  std::unique_ptr<VImage> m_depth;
  std::shared_ptr<VQueue> m_graphicsQueue, m_presentQueue;
  VPooledBuffers m_bufferPool;
  std::unique_ptr<VCmdBuffer> m_cmdInit;
  std::unique_ptr<VRenderPass> m_renderPass;
  std::unique_ptr<VSwapchain> m_swapchain;
  std::unique_ptr<VSemaphore> m_imageAvailable;
  VPooledPipelines m_pipelinePool;
  std::unique_ptr<VDescriptorPool> m_descriptorPool;
  std::shared_ptr<VViewportState> m_viewportState;
  std::shared_ptr<VMultisampleState> m_multisampleState;
  std::shared_ptr<VDynamicState> m_dynamicState;
  std::unique_ptr<VImage> m_stagingImage;
  std::unique_ptr<VBuffer> m_stagingBuffer;
  std::vector<std::shared_ptr<VModelInstance>> m_toRender;
  std::vector<VkClearValue> m_clearValues;
  uint64_t m_frame = 100; // start from a big number so we don't have to worry about subtracting from this number when we manage releasing of resources from previous frames
  uint64_t m_frameInvalidated = 0;

  VDevice(VGraphics &graphics);

  uint64_t GetSafeReleaseFrame() const { return m_frame - m_swapchain->m_surfaces.size(); }

  VImage *LoadVImage(std::string const &filename);
  VBuffer *LoadVBuffer(uint64_t size, VkBufferUsageFlags usage, void *data);
  VShader *LoadVShader(std::string const &filename);

  void UpdateStagingImage(uint32_t width, uint32_t height, uint32_t depth, uint32_t components);
  void FreeStagingImage();

  void UpdateStagingBuffer(uint64_t size);
  void FreeStagingBuffer();

  void SetClearColor(float r, float g, float b, float a);
  void SetClearDepthStencil(float depth, uint32_t stencil);

  void InitSwapchain(uint32_t width, uint32_t height);
  
  void Add(std::shared_ptr<VModelInstance> instance);

  void RenderFrame();

  void WaitIdle();
public:
  void InitCapabilities();
  void InitDevice();
  void InitDepth(uint32_t width, uint32_t height);
  void InitCmdBuffers(bool synchronize, uint32_t count);
  void InitState();

  void SubmitInitCommands();

  void UpdateToRender();
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
  unsigned m_graphicsQueueFamily, m_presentQueueFamily;
  UniqueResource<VkSurfaceKHR> m_surface;
  VkSurfaceFormatKHR m_surfaceFormat;
  VkFormat m_depthFormat;
  std::unique_ptr<VDevice> m_device;

  std::vector<std::string> m_layerNames, m_extensionNames;
  UniqueResource<VkDebugReportCallbackEXT> m_debugReportCallback;

  PFN_vkCreateDebugReportCallbackEXT  m_CreateDebugReportCallbackEXT = nullptr;
  PFN_vkDestroyDebugReportCallbackEXT m_DestroyDebugReportCallbackEXT = nullptr;
  PFN_vkDebugReportMessageEXT         m_DebugReportMessageEXT = nullptr;

  VGraphics(bool validate, std::string const &appName, uint32_t appVersion, uintptr_t instanceID, uintptr_t windowID);

  bool CompatibleOptimalFormat(VkFormat format, VkFormatFeatureFlagBits requiredFeatures);
  
  bool IsPipelineCacheDataCompatible(std::vector<uint8_t> const &data);

public:
  void InitInstance(std::string const &appName, uint32_t appVersion);
  void InitPhysicalDevice();
  void InitSurface(uintptr_t instanceID, uintptr_t windowID);
  void InitDepthFormat();

  uint32_t GetMemoryTypeIndex(uint32_t validTypeMask, VkMemoryPropertyFlags flags);

  static bool AddLayerName(std::vector<VkLayerProperties> const &layers, std::string const &name, std::vector<std::string> &layerNames);
  static bool AddExtensionName(std::vector<VkExtensionProperties> const &extensions, std::string const &name, std::vector<std::string> &extNames);
  static bool AppendExtensionProperties(std::vector<VkExtensionProperties> &extensions, std::string const &layerName);

  static std::vector<char const*> GetPtrArray(std::vector<std::string> const &strs);

  static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);
};

