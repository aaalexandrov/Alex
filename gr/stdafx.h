#pragma once

#include <algorithm>
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

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_projection.hpp"

#include "rttr/rttr_enable.h"
#include "rttr/rttr_cast.h"
#include "rttr/registration.h"

#include "util/namespace.h"
#include "util/dbg.h"
#include "util/rtti.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "vulkan/vulkan.hpp"

#undef CreateWindow
#undef CreateSemaphore
#undef LoadImage



