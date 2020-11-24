#pragma once

#include <malloc.h>
#include <functional>
#include "namespace.h"

NAMESPACE_BEGIN(util)

template <typename Elem, size_t N>
constexpr size_t ArraySize(Elem(&)[N])
{
  return N;
}

inline size_t MemSize(void *mem)
{
  if (!mem)
    return 0;
  size_t size;
#if defined(_WIN32)
  size = _msize(mem);
#else
  size = malloc_usable_size(mem);
#endif
  return size;
}

template <typename Class, typename MemType>
size_t MemberOffset(MemType Class::*memPtr)
{
  return reinterpret_cast<uint8_t*>(&(static_cast<Class*>(nullptr)->*memPtr)) - static_cast<uint8_t*>(nullptr);
}

template <typename Cls>
struct SizeAlign {
  static constexpr size_t SizeOf() { return sizeof(Cls); }
  static constexpr size_t AlignOf() { return alignof(Cls); }
};

template <>
struct SizeAlign<void> {
  static constexpr size_t SizeOf() { return 0; }
  static constexpr size_t AlignOf() { return 1; }
};

template <typename Resource>
struct AutoFree {
  using FreeFunc = std::function<void(Resource)>;

  Resource _data;
  FreeFunc _free;

  AutoFree(Resource data, FreeFunc free) : _data { data }, _free { free } {}
  ~AutoFree() { Free(); }

  void Free() 
  {
    if (_free) {
      _free(_data);
      _free = nullptr;
    }
  }

  Resource Get() const { return _data; }
};

template <typename TRAITS>
struct UniqueHandle {
  using Handle = typename TRAITS::Handle;
  using Owner = typename TRAITS::Owner;

  UniqueHandle(Handle handle, Owner owner = TRAITS::NullOwner()) : _owner(owner), _handle(handle) {}

  UniqueHandle() = default;
  UniqueHandle(UniqueHandle const &other) = delete;
  UniqueHandle(UniqueHandle &&other) : _owner(other.GetOwner()), _handle(other.Release()) {}
  ~UniqueHandle() { Destroy(); }

  Owner GetOwner() { return _owner; }

  Handle operator *() { return _handle; }
  Handle operator ->() { return _handle; }

  UniqueHandle &operator =(UniqueHandle const &other) = delete;
  UniqueHandle &operator =(UniqueHandle &&other) 
  {
    Reset(other.Release());
    _owner = other.GetOwner();
    return *this; 
  }

  explicit operator bool() { return !!_handle; }

  Handle Release() 
  { 
    Handle handle = _handle; 
    _handle = TRAITS::NullHandle(); 
    return handle; 
  }

  void Destroy() 
  { 
    if (_handle != TRAITS::NullHandle()) {
      TRAITS::Destroy(_owner, _handle);
      _handle = TRAITS::NullHandle();
    }
  }

  void Reset(Handle handle) 
  { 
    if (handle != _handle) {
      Destroy(); 
      _handle = handle;
    }
  }

  Owner _owner = TRAITS::NullOwner();
  Handle _handle = TRAITS::NullHandle();
};

NAMESPACE_END(util)