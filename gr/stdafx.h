#pragma once

#include <algorithm>
#include <execution>
#include <functional>
#include <string>
#include <mutex>
#include <chrono>
#include <memory>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <map>

#include "rttr/rttr_enable.h"
#include "rttr/rttr_cast.h"
#include "rttr/registration.h"

#include "util/namespace.h"
#include "util/dbg.h"
#include "util/rtti.h"
#include "util/mathutl.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "vulkan/vulkan.hpp"

#undef CreateWindow
#undef CreateSemaphore
#undef LoadImage



