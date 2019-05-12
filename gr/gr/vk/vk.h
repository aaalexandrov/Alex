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

#include "gr/vk/vma.h"
#include "shaderc/libshaderc/include/shaderc/shaderc.hpp"
#include "SPIRV-Cross/spirv_reflect.hpp"

#include "util/namespace.h"