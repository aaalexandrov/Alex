#pragma once

#include <malloc.h>
#include "namespace.h"

NAMESPACE_BEGIN(util)

inline size_t MemSize(void *mem)
{
  if (!mem)
    return 0;
  size_t size;
#if defined(_WIN32)
  size = _msize(mem);
#else
  size = malloc_size(mem);
#endif
  return size;
}


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