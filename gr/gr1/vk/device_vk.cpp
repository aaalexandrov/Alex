#include "device_vk.h"
#include "../graphics_exception.h"
#include "../render_pass.h"
#include "execution_queue_vk.h"
#include "shader_vk.h"
#include "render_state_vk.h"
#include "rttr/registration.h"
#include "util/mathutl.h"
#include "util/enumutl.h"

#if defined(_WIN32)
#include "../win32//presentation_surface_create_data_win32.h"
#elif defined(__linux__)
#include "../x11/presentation_surface_create_data_xlib.h"
#endif

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<DeviceVk>("DeviceVk")
		.constructor<Host::DeviceInfo const &, PresentationSurfaceCreateData const *, ValidationLevel>()(rttr::policy::ctor::as_std_shared_ptr);

	registration::class_<HostPlatformVk>("HostPlatformVk")
		.constructor<>()(rttr::policy::ctor::as_std_shared_ptr);
}

HostPlatformVk::~HostPlatformVk()
{
}

void HostPlatformVk::GetSupportedDevices(std::vector<Host::DeviceInfo> &deviceInfos)
{
	vk::InstanceCreateInfo instInfo;
	auto inst = vk::createInstanceUnique(instInfo);
	std::vector<vk::PhysicalDevice> physDevices = inst->enumeratePhysicalDevices();

	for (auto device : physDevices) {
		vk::PhysicalDeviceProperties devProps = device.getProperties();
		
		Host::DeviceInfo info;
		info._name = (char const *)devProps.deviceName;
		info._version = DeviceVk::VersionToVector(devProps.apiVersion);
		deviceInfos.emplace_back(std::move(info));
	}
}

std::shared_ptr<Device> HostPlatformVk::CreateDevice(Host::DeviceInfo const &deviceInfo, PresentationSurfaceCreateData const *surfaceData, ValidationLevel validation)
{
	return std::make_shared<DeviceVk>(deviceInfo, surfaceData, validation);
}


DeviceVk::DeviceVk(Host::DeviceInfo const &deviceInfo, PresentationSurfaceCreateData const *surfaceData, ValidationLevel validation)
	: Device(deviceInfo, validation, rttr::type::get<DeviceVk>())
{
	CreateInstance();
	CreatePhysicalDevice(surfaceData);
	CreateDevice();

	_pipelineStore.Init(*this);
	_queue = std::make_unique<ExecutionQueueVk>(*this);
}

DeviceVk::~DeviceVk()
{
	_queue.reset();
}

void DeviceVk::WaitIdle()
{
	_device->waitIdle();
}

glm::uvec3 DeviceVk::VersionToVector(uint32_t version)
{
	return glm::uvec3(VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
}

vk::UniqueSemaphore DeviceVk::CreateSemaphore()
{
	vk::SemaphoreCreateInfo semInfo;
	auto semaphore = _device->createSemaphoreUnique(semInfo, AllocationCallbacks());
	return std::move(semaphore);
}

vk::UniqueFence DeviceVk::CreateFence(bool createSignaled)
{
	vk::FenceCreateInfo fenceInfo(createSignaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags());
	return _device->createFenceUnique(fenceInfo, AllocationCallbacks());
}

void DeviceVk::CreateInstance()
{
	auto instanceLayers = vk::enumerateInstanceLayerProperties();
	auto instanceExtensions = vk::enumerateInstanceExtensionProperties();

	std::vector<char const*> layerNames;
	std::vector<char const *> extensionNames;

	if (_validationLevel > ValidationLevel::None) {
#if defined(_WIN32)		
		//AppendLayer(layerNames, instanceLayers, "VK_LAYER_LUNARG_standard_validation");
#endif		
		AppendLayer(layerNames, instanceLayers, "VK_LAYER_KHRONOS_validation");
		AppendExtension(extensionNames, instanceExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	AppendExtension(extensionNames, instanceExtensions, VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
	AppendExtension(extensionNames, instanceExtensions, VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	AppendExtension(extensionNames, instanceExtensions, VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
	#error Unsupported platform!
#endif
	
	vk::InstanceCreateInfo instanceInfo;
	instanceInfo
		.setEnabledExtensionCount(static_cast<uint32_t>(extensionNames.size()))
		.setPpEnabledExtensionNames(extensionNames.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layerNames.size()))
		.setPpEnabledLayerNames(layerNames.data());

	vk::DebugReportCallbackCreateInfoEXT debugCBInfo(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError,
		DbgReportFunc, this);

	if (_validationLevel > ValidationLevel::None) {
		instanceInfo.pNext = &debugCBInfo;

		_hostAllocTracker = std::make_unique<HostAllocationTrackerVk>();
	}

	_instance = vk::createInstanceUnique(instanceInfo);

	_dynamicDispatch.init(*_instance, vkGetInstanceProcAddr);

	if (_validationLevel > ValidationLevel::None) {
		_debugReportCallback = _instance->createDebugReportCallbackEXTUnique(debugCBInfo, AllocationCallbacks(), _dynamicDispatch);
	}
}

int32_t DeviceVk::GetSuitableQueueFamily(std::vector<vk::QueueFamilyProperties> &queueProps, QueueRole role, PresentationSurfaceCreateData const *surfaceData)
{
	vk::QueueFlags flags;
	switch (role) {
		case QueueRole::Graphics:
			flags = vk::QueueFlagBits::eGraphics; break;
		case QueueRole::Compute:
			flags = vk::QueueFlagBits::eCompute; break;
		case QueueRole::Transfer:
			flags = vk::QueueFlagBits::eTransfer; break;
		case QueueRole::SparseOp:
			flags = vk::QueueFlagBits::eSparseBinding; break;
		case QueueRole::Present: 
			break;
		default: ASSERT(!"Unsupported queue role!");
	}

	auto familySupportsPresent = [this, surfaceData](int32_t q)->bool {
#if defined(_WIN32)
		return _physicalDevice.getWin32PresentationSupportKHR(q);
#elif defined (__linux__)
		auto createXlib = rttr::rttr_cast<PresentationSurfaceCreateDataXlib const *>(surfaceData);
		return _physicalDevice.getXlibPresentationSupportKHR(q, createXlib->_display, createXlib->GetVisualId());
#else
#error Unsupported platform!
#endif
	};

	int32_t best = -1;
	for (auto i = 0; i < queueProps.size(); ++i) {
		auto &queue = queueProps[i];
		if ((queue.queueFlags & flags) != flags || role == QueueRole::Present && !familySupportsPresent(i))
			continue;

		const vk::QueueFlags graphicsCompute = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute;

		if (best < 0 ||
			// we prefer a present queue that has compute or graphics to work around drivers erroneously returning present support on transfer queues, which fail to actually present
			role == QueueRole::Present && (queue.queueFlags & graphicsCompute) && !(queueProps[best].queueFlags & graphicsCompute) ||
			// a queue with less overall capabilities is better, that way we get a more dedicated queue for the function
			util::CountSetBits(static_cast<uint32_t>(queue.queueFlags)) < util::CountSetBits(static_cast<uint32_t>(queueProps[best].queueFlags)) && 
			!(role == QueueRole::Present && !(queue.queueFlags & graphicsCompute) && (queueProps[best].queueFlags & graphicsCompute))) {
			best = i;
		}
	}

	return best;
}

void DeviceVk::CreatePhysicalDevice(PresentationSurfaceCreateData const *surfaceData)
{
	auto physDevices = _instance->enumeratePhysicalDevices();
	auto it = std::find_if(physDevices.begin(), physDevices.end(), [this](vk::PhysicalDevice physDev) {
		vk::PhysicalDeviceProperties physDevProps = physDev.getProperties();
		return _deviceInfo._name == physDevProps.deviceName && _deviceInfo._version == VersionToVector(physDevProps.apiVersion);
	});
	_physicalDevice = *it;

	std::vector<vk::QueueFamilyProperties> queueProps = _physicalDevice.getQueueFamilyProperties();

	for (QueueRole role = QueueRole::First; role <= QueueRole::Last; role = util::EnumInc(role)) {
		QueueData(role)._family = GetSuitableQueueFamily(queueProps, role, surfaceData);
	}
}

void DeviceVk::CreateDevice()
{
	auto deviceLayers = _physicalDevice.enumerateDeviceLayerProperties();
	auto deviceExtensions = _physicalDevice.enumerateDeviceExtensionProperties();

	std::vector<char const *> layerNames, extensionNames;

	if (_validationLevel > ValidationLevel::None) {
#if defined(_WIN32)		
		//AppendLayer(layerNames, deviceLayers, "VK_LAYER_LUNARG_standard_validation");
#endif		
	}

	AppendExtension(extensionNames, deviceExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	std::vector<vk::DeviceQueueCreateInfo> queuesInfo;
	std::vector<float> queuesPriorities;

	InitQueueFamiliesInfo(queuesInfo, queuesPriorities);

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo
		.setQueueCreateInfoCount(static_cast<int32_t>(queuesInfo.size()))
		.setPQueueCreateInfos(queuesInfo.data())
		.setEnabledLayerCount(static_cast<int32_t>(layerNames.size()))
		.setPpEnabledLayerNames(layerNames.data())
		.setEnabledExtensionCount(static_cast<int32_t>(extensionNames.size()))
		.setPpEnabledExtensionNames(extensionNames.data());

	_device = _physicalDevice.createDeviceUnique(deviceInfo, AllocationCallbacks());

	_allocator = VmaAllocatorCreateUnique(_physicalDevice, *_device, AllocationCallbacks());

	InitQueues(queuesInfo);
}

void DeviceVk::InitQueueFamiliesInfo(std::vector<vk::DeviceQueueCreateInfo> &queuesInfo, std::vector<float> &queuesPriorities)
{
	if (QueueData(QueueRole::Present)._family < 0)
		throw GraphicsException("Physical device presentation queue family not initialized, you need to create a surface first!", VK_RESULT_MAX_ENUM);

	std::vector<int32_t> families;

	for (QueueRole role = QueueRole::First; role <= QueueRole::Last; role = util::EnumInc(role)) {
		int32_t familyForRole = QueueData(role)._family;
		if (familyForRole >= 0)
			families.push_back(familyForRole);
	}

	// reserve priority array so it doesn't get reallocated while we add to it
	queuesPriorities.reserve(families.size());

	std::sort(families.begin(), families.end());

	std::vector<vk::QueueFamilyProperties> queueProps = _physicalDevice.getQueueFamilyProperties();

	for (int32_t i = 0; i < families.size(); ) {
		int32_t endRange = i + 1;
		for (; endRange < families.size() && families[i] == families[endRange]; ++endRange);
		int32_t count = std::min<int32_t>(queueProps[families[i]].queueCount, endRange - i);

		// initialize with equal priority of 0
		std::fill_n(std::back_inserter(queuesPriorities), count, 0.0f);
		queuesInfo.emplace_back(vk::DeviceQueueCreateFlags(), families[i], count, &queuesPriorities[queuesPriorities.size() - count]);

		i = endRange;
	}
}

void DeviceVk::InitQueues(std::vector<vk::DeviceQueueCreateInfo> const &queuesInfo)
{
	struct QueueFamilyData {
		int32_t _startIndex, _count;
		int32_t _usedCount = 0;
	};
	std::unordered_map<int32_t, QueueFamilyData> queueFamilyData;

	int32_t numQueues = 0;
	for (auto &qi : queuesInfo) {
		for (int32_t i = 0; i < int32_t(qi.queueCount); ++i) {
			_queues[numQueues + i].Init(*this, qi.queueFamilyIndex, i);
		}
		queueFamilyData[qi.queueFamilyIndex] = {numQueues, (int32_t)qi.queueCount};
		numQueues += qi.queueCount;
	}

	for (QueueRole role = QueueRole::First; role <= QueueRole::Last; role = util::EnumInc(role)) {
		QueueFamilyData &familyData = queueFamilyData[QueueData(role)._family];
		QueueData(role)._queueIndex = familyData._startIndex + (familyData._usedCount % familyData._count);
		familyData._usedCount++;
	}
}

vk::LayerProperties const *DeviceVk::GetLayer(std::vector<vk::LayerProperties> const &layers, std::string const &layerName)
{
	auto layerIt = std::find_if(layers.begin(), layers.end(),
		[&](vk::LayerProperties const& layer)->bool { return layerName == layer.layerName; });
	if (layerIt == layers.end())
		return nullptr;
	return &*layerIt;
}

vk::ExtensionProperties const *DeviceVk::GetExtension(std::vector<vk::ExtensionProperties> const &extensions, std::string const &extensionName)
{
	auto extIt = std::find_if(extensions.begin(), extensions.end(),
		[&](vk::ExtensionProperties const& extension)->bool { return extensionName == extension.extensionName; });
	if (extIt == extensions.end())
		return nullptr;
	return &*extIt;
}

void DeviceVk::AppendLayer(std::vector<char const *> &layers, std::vector<vk::LayerProperties> const &availableLayers, std::string const &layerName)
{
	auto layer = GetLayer(availableLayers, layerName);
	if (!layer)
		throw GraphicsException("GraphicsVk::AppendLayer() failed to find layer " + layerName, VK_RESULT_MAX_ENUM);
	layers.push_back(layer->layerName);
}

void DeviceVk::AppendExtension(std::vector<char const *> &extensions, std::vector<vk::ExtensionProperties> const &availableExtensions, std::string const &extensionName)
{
	auto ext = GetExtension(availableExtensions, extensionName);
	if (!ext)
		throw GraphicsException("GraphicsVk::AppendExtension() failed to find extension " + extensionName, VK_RESULT_MAX_ENUM);
	extensions.push_back(ext->extensionName);
}

vk::AllocationCallbacks *DeviceVk::AllocationCallbacks()
{
	if (!_hostAllocTracker)
		return nullptr;
	return &_hostAllocTracker->_allocCallbacks;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DeviceVk::DbgReportFunc(
	VkFlags msgFlags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char *pLayerPrefix,
	const char *pMsg,
	void *pUserData)
{
	std::string flag = (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) ? "error" : "warning";

	LOG("Vulkan debug ", flag, ", code: ", msgCode, ", layer: ", pLayerPrefix, ", msg: ", pMsg);

	return false;
}


NAMESPACE_END(gr1)

