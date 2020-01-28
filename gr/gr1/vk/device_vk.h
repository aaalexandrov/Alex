#pragma once

#include "../device.h"
#include "../host.h"
#include "host_allocation_tracker_vk.h"
#include "vk.h"
#include "queue_vk.h"

NAMESPACE_BEGIN(gr1)

class HostPlatformVk : public HostPlatform {
	RTTR_ENABLE(HostPlatform)
public:
	~HostPlatformVk() override;
	void GetSupportedDevices(std::vector<Host::DeviceInfo> &deviceInfos) override;
	std::shared_ptr<Device> CreateDevice(Host::DeviceInfo const &deviceInfo, ValidationLevel validation) override;
};

class DeviceVk : public Device {
	RTTR_ENABLE(Device)
public:
	DeviceVk(Host::DeviceInfo const &deviceInfo, ValidationLevel validation);
	~DeviceVk() override;

	static glm::uvec3 VersionToVector(uint32_t version);

	vk::AllocationCallbacks *AllocationCallbacks();

	inline QueueVk &Queue(QueueRole role) { return _queues[static_cast<int>(role)]; }

	inline QueueVk &GraphicsQueue() { return Queue(QueueRole::Graphics); }
	inline QueueVk &PresentQueue() { return Queue(QueueRole::Present); }
	inline QueueVk &TransferQueue() { return Queue(QueueRole::Transfer); }
	inline QueueVk &ComputeQueue() { return Queue(QueueRole::Compute); }
	inline QueueVk &SparseOpQueue() { return Queue(QueueRole::SparseOp); }

	vk::UniqueSemaphore CreateSemaphore();
	vk::UniqueFence CreateFence(bool createSignaled = false);

protected:
	void CreateInstance();
	void CreatePhysicalDevice();
	void CreateDevice();

	inline int32_t &QueueFamilyIndex(QueueRole role) { return _queueFamilyIndices[static_cast<int>(role)]; }
	void InitQueueFamiliesInfo(std::vector<vk::DeviceQueueCreateInfo> &queuesInfo, std::vector<float> &queuesPriorities);
	void InitQueues();

	static vk::LayerProperties const *GetLayer(std::vector<vk::LayerProperties> const &layers, std::string const &layerName);
	static vk::ExtensionProperties const *GetExtension(std::vector<vk::ExtensionProperties> const &extensions, std::string const &extensionName);

	static void AppendLayer(std::vector<char const *> &layers, std::vector<vk::LayerProperties> const &availableLayers, std::string const &layerName);
	static void AppendExtension(std::vector<char const *> &extensions, std::vector<vk::ExtensionProperties> const &availableExtensions, std::string const &extensionName);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DbgReportFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData);

	static int32_t GetSuitableQueueFamily(std::vector<vk::QueueFamilyProperties> queueProps, vk::QueueFlags flags, std::function<bool(int32_t)> predicate = [](auto i) {return true; });

	// Allocation tracker needs to appear before any Vulkan RAII resources
	// Resources will be calling it during destruction, so the tracker has to outlive them
	std::unique_ptr<HostAllocationTrackerVk> _hostAllocTracker;

	vk::DispatchLoaderDynamic _dynamicDispatch;

public:
	vk::UniqueInstance _instance;
	vk::PhysicalDevice _physicalDevice;

	using UniqueDebugReportCallbackEXT = vk::UniqueHandle<vk::DebugReportCallbackEXT, vk::DispatchLoaderDynamic>;
	UniqueDebugReportCallbackEXT _debugReportCallback;

	std::array<int32_t, static_cast<int>(QueueRole::Count)> _queueFamilyIndices;
	vk::UniqueDevice _device;
	UniqueVmaAllocator _allocator;
	std::array<QueueVk, static_cast<int>(QueueRole::Count)> _queues;
};

NAMESPACE_END(gr1)

