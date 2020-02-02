#include "presentation_surface_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "../execution_queue.h"
#include "../graphics_exception.h"
#include "util/mathutl.h"
#include "rttr/registration.h"
#include "rttr/rttr_cast.h"

#if defined(_WIN32)
#include "../win32/presentation_surface_create_data_win32.h"
#elif defined(linux)
#error Unimplemented
#else
#error Unsupported platform
#endif

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<PresentationSurfaceVk>("PresentationSurfaceVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(rttr::policy::ctor::as_raw_ptr);
}

void PresentationSurfaceVk::Init(PresentationSurfaceCreateData &createData, PresentMode presentMode)
{
	PresentationSurface::Init(createData, presentMode);
#if defined(_WIN32)
	InitSurfaceWin32(&createData);
#elif defined(linux)
	InitSurfaceXlib(&createData);
#endif

	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	bool surfaceSupported = deviceVk->_physicalDevice.getSurfaceSupportKHR(deviceVk->PresentQueue()._family, *_surface);
	if (!surfaceSupported)
		throw GraphicsException("CreateSwapChain() failed, surface not supported for presentation on this device!", VK_ERROR_INITIALIZATION_FAILED);

	_acquireSemaphore = deviceVk->CreateSemaphore();

	_surfaceFormat = GetSurfaceFormat();
}

#if defined(_WIN32)

void PresentationSurfaceVk::InitSurfaceWin32(PresentationSurfaceCreateData *createData)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	auto createWin32 = rttr::rttr_cast<PresentationSurfaceCreateDataWin32*>(createData);
	vk::Win32SurfaceCreateInfoKHR surfaceInfo(vk::Win32SurfaceCreateFlagsKHR(), createWin32->_hInstance, createWin32->_hWnd);
	_surface = deviceVk->_instance->createWin32SurfaceKHRUnique(surfaceInfo, deviceVk->AllocationCallbacks());
}

#elif defined(linux)

void PresentationSurfaceVk::InitSurfaceXlib(PresentationSurfaceCreateData *createData)
{
#error Unimplemented
}

#endif

void PresentationSurfaceVk::Update(uint32_t width, uint32_t height)
{
	CreateSwapChain(width, height);
}

glm::uvec2 PresentationSurfaceVk::GetSize()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	vk::SurfaceCapabilitiesKHR surfaceCaps = deviceVk->_physicalDevice.getSurfaceCapabilitiesKHR(*_surface);
	return glm::uvec2(surfaceCaps.currentExtent.width, surfaceCaps.currentExtent.height);
}

std::shared_ptr<Image> const &PresentationSurfaceVk::AcquireNextImage()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	uint64_t timeout = std::numeric_limits<uint64_t>::max();
	auto result = deviceVk->_device->acquireNextImageKHR(*_swapchain, timeout, *_acquireSemaphore, nullptr);
	if (result.result != vk::Result::eSuccess)
		throw GraphicsException("AcquireNextImage() failed!", (uint32_t)result.result);
	uint32_t imageIndex = result.value;
	_images[imageIndex]->SetResourceState(ResourceState::PresentAcquired);
	return _images[imageIndex];
}

void PresentationSurfaceVk::CreateSwapChain(uint32_t width, uint32_t height)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	vk::SurfaceCapabilitiesKHR surfaceCaps = deviceVk->_physicalDevice.getSurfaceCapabilitiesKHR(*_surface);
	vk::Extent2D extent = GetExtent(surfaceCaps, width, height);
	uint32_t imageCount = surfaceCaps.minImageCount + 1;
	if (surfaceCaps.maxImageCount > 0)
		imageCount = std::min(imageCount, surfaceCaps.maxImageCount);

	vk::SwapchainCreateInfoKHR chainInfo;
	chainInfo
		.setSurface(*_surface)
		.setMinImageCount(imageCount)
		.setImageFormat(_surfaceFormat.format)
		.setImageColorSpace(_surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setPreTransform(
		(surfaceCaps.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			? vk::SurfaceTransformFlagBitsKHR::eIdentity
			: surfaceCaps.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(GetVkPresentMode())
		.setClipped(true);

	_images.clear();
	_swapchain.reset();

	_swapchain = deviceVk->_device->createSwapchainKHRUnique(chainInfo, deviceVk->AllocationCallbacks());

	 std::vector<vk::Image> chainImages = deviceVk->_device->getSwapchainImagesKHR(*_swapchain);
	 for (auto img : chainImages) {
		 _images.push_back(deviceVk->CreateResource<Image>());
		 ImageVk *imageVk = static_cast<ImageVk*>(_images.back().get());
		 glm::uvec2 size = GetSize();
		 imageVk->Init(this, Image::Usage::RenderTarget, img, GetSurfaceFormat().format, glm::uvec4(size.x, size.y, 0, 0), 1);
	 }
}

vk::SurfaceFormatKHR PresentationSurfaceVk::GetSurfaceFormat()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	std::vector<vk::SurfaceFormatKHR> formats = deviceVk->_physicalDevice.getSurfaceFormatsKHR(*_surface);
	auto format = formats[0];
	if (format.format == vk::Format::eUndefined)
		format.format = vk::Format::eR8G8B8A8Unorm;
	return format;
}

util::ValueRemapper<vk::PresentModeKHR, PresentMode> PresentationSurfaceVk::s_vk2PresentMode{ {
		{vk::PresentModeKHR::eImmediate, PresentMode::Immediate },
		{vk::PresentModeKHR::eMailbox, PresentMode::Mailbox },
		{vk::PresentModeKHR::eFifo, PresentMode::Fifo },
		{vk::PresentModeKHR::eFifoRelaxed, PresentMode::FifoRelaxed },
	} };

vk::PresentModeKHR PresentationSurfaceVk::GetVkPresentMode()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	vk::PresentModeKHR mode = s_vk2PresentMode.ToSrc(_presentMode);
	std::vector<vk::PresentModeKHR> modes = deviceVk->_physicalDevice.getSurfacePresentModesKHR(*_surface);
	if (std::find(modes.begin(), modes.end(), mode) == modes.end()) {
		ASSERT(!"Requested present mode not found, defaulting to first available!");
		mode = modes[0];
	}
	return mode;
}

uint32_t PresentationSurfaceVk::GetImageIndex(std::shared_ptr<Image> const &image)
{
	for (uint32_t i = 0; i < _images.size(); ++i) {
		if (image == _images[i])
			return i;
	}
	return ~0;
}

vk::Extent2D PresentationSurfaceVk::GetExtent(vk::SurfaceCapabilitiesKHR const &surfaceCaps, uint32_t width, uint32_t height)
{
	if (surfaceCaps.currentExtent != std::numeric_limits<uint32_t>::max())
		return surfaceCaps.currentExtent;

	vk::Extent2D ext{
		util::Clamp(surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width, width),
		util::Clamp(surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height, height)
	};

	return ext;
}

NAMESPACE_END(gr1)