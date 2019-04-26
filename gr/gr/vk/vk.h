#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(linux)
#define VK_USE_PLATFORM_XLIB_KHR
#else
#error Unsupported platform!
#endif

#include "vulkan/vulkan.hpp"

#undef min
#undef max