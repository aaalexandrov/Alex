#pragma once

#if defined(_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
#elif defined(__linux__)
	#define VK_USE_PLATFORM_XLIB_KHR
#else
	#error Unsupported platform!
#endif

#include "vulkan/vulkan.hpp"

#include "vma.h"
#include "shaderc/shaderc.hpp"
#include "spirv_cross/spirv_reflect.hpp"

#if defined(_WIN32)
	#undef CreateWindow
	#undef CreateSemaphore
	#undef LoadImage
#elif defined(__linux__)
	#undef None
	#undef Always
#endif

#include "util/namespace.h"

NAMESPACE_BEGIN(gr1)

template <typename Owner, typename UniqueHandle>
class OwnedUniqueHandle
{
public:
	using HandleType = typename UniqueHandle::element_type;

	OwnedUniqueHandle(Owner *owner = nullptr, UniqueHandle &&handle = UniqueHandle()) noexcept : _handle(std::move(handle)), _owner(owner) {}
	OwnedUniqueHandle(OwnedUniqueHandle &&other) noexcept : OwnedUniqueHandle(other._owner, std::move(other._handle)) {}

	~OwnedUniqueHandle() { reset(); }

	void lock() noexcept
	{
		if (_owner && _handle)
			_owner->_mutex.lock();
	}

	void unlock() noexcept
	{
		if (_owner && _handle)
			_owner->_mutex.unlock();
	}

	bool try_lock() noexcept
	{
		if (!_owner || !_handle)
			return true;
		return _owner->_mutex.try_lock();
	}

	void reset() noexcept 
	{ 
		if (_owner && _handle) {
			std::lock_guard<std::recursive_mutex> lock(_owner->_mutex);
			_handle.reset();
		}
	}
	OwnedUniqueHandle release() noexcept { return _handle.release(); }
	explicit operator bool() const noexcept { return (bool)_handle; }

	HandleType *operator->() noexcept { return _handle.operator->(); }
	HandleType const *operator->() const noexcept { return _handle.operator->(); }

	HandleType &operator*() noexcept { return *_handle; }
	HandleType const &operator*() const noexcept { return *_handle; }

	OwnedUniqueHandle &operator=(OwnedUniqueHandle &&other) noexcept 
	{
		reset();
		_owner = other._owner;
		_handle = std::move(other._handle);
		return *this;
	}
private:
	Owner *_owner;
	UniqueHandle _handle;
};

NAMESPACE_END(gr1)