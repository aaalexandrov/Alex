#pragma once

#include "../device.h"
#include "../host.h"
#include "../shader.h"
#include "../render_pass.h"
#include "host_allocation_tracker_vk.h"
#include "queue_vk.h"
#include "descriptor_set_store.h"
#include "pipeline_store.h"
#include "util/layout.h"

NAMESPACE_BEGIN(gr1)

class HostPlatformVk : public HostPlatform {
	RTTR_ENABLE(HostPlatform)
public:
	~HostPlatformVk() override;
	void GetSupportedDevices(std::vector<Host::DeviceInfo> &deviceInfos) override;
	std::shared_ptr<Device> CreateDevice(Host::DeviceInfo const &deviceInfo, PresentationSurfaceCreateData const *surfaceData, ValidationLevel validation) override;
};

class DeviceVk : public Device {
	RTTR_ENABLE(Device)
public:
	DeviceVk(Host::DeviceInfo const &deviceInfo, PresentationSurfaceCreateData const *surfaceData, ValidationLevel validation);
	~DeviceVk() override;

	void WaitIdle() override;

	static glm::uvec3 VersionToVector(uint32_t version);

	vk::AllocationCallbacks *AllocationCallbacks();

	inline QueueVk &Queue(QueueRole role) { return _queues[_queueRoleData[static_cast<int>(role)]._queueIndex]; }

	inline QueueVk &GraphicsQueue() { return Queue(QueueRole::Graphics); }
	inline QueueVk &PresentQueue() { return Queue(QueueRole::Present); }
	inline QueueVk &TransferQueue() { return Queue(QueueRole::Transfer); }
	inline QueueVk &ComputeQueue() { return Queue(QueueRole::Compute); }
	inline QueueVk &SparseOpQueue() { return Queue(QueueRole::SparseOp); }

	vk::UniqueSemaphore CreateSemaphore();
	vk::UniqueFence CreateFence(bool createSignaled = false);

protected:
	void CreateInstance();
	void CreatePhysicalDevice(PresentationSurfaceCreateData const *surfaceData);
	void CreateDevice();

	struct QueueRoleData {
		int32_t _family = -1;
		int32_t _queueIndex = -1;
	};

	inline QueueRoleData &QueueData(QueueRole role) { return _queueRoleData[static_cast<int>(role)]; }
	void InitQueueFamiliesInfo(std::vector<vk::DeviceQueueCreateInfo> &queuesInfo, std::vector<float> &queuesPriorities);
	void InitQueues(std::vector<vk::DeviceQueueCreateInfo> const &queuesInfo);

	int32_t GetSuitableQueueFamily(std::vector<vk::QueueFamilyProperties> &queueProps, QueueRole role, PresentationSurfaceCreateData const *surfaceData);

	static vk::LayerProperties const *GetLayer(std::vector<vk::LayerProperties> const &layers, std::string const &layerName);
	static vk::ExtensionProperties const *GetExtension(std::vector<vk::ExtensionProperties> const &extensions, std::string const &extensionName);

	static void AppendLayer(std::vector<char const *> &layers, std::vector<vk::LayerProperties> const &availableLayers, std::string const &layerName);
	static void AppendExtension(std::vector<char const *> &extensions, std::vector<vk::ExtensionProperties> const &availableExtensions, std::string const &extensionName);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);


	// Allocation tracker needs to appear before any Vulkan RAII resources
	// Resources will be calling it during destruction, so the tracker has to outlive them
	std::unique_ptr<HostAllocationTrackerVk> _hostAllocTracker;

	vk::DispatchLoaderDynamic _dynamicDispatch;

public:
	vk::UniqueInstance _instance;
	vk::PhysicalDevice _physicalDevice;

	using UniqueDebugReportCallbackEXT = vk::UniqueHandle<vk::DebugReportCallbackEXT, vk::DispatchLoaderDynamic>;
	UniqueDebugReportCallbackEXT _debugReportCallback;

	vk::UniqueDevice _device;
	UniqueVmaAllocator _allocator;
	std::array<QueueRoleData, static_cast<int>(QueueRole::Count)> _queueRoleData;
	std::array<QueueVk, static_cast<int>(QueueRole::Count)> _queues;
	PipelineStore _pipelineStore;
};

NAMESPACE_END(gr1)

