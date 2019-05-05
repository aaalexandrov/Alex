#pragma once

#include "namespace.h"
#include <mutex>
#include <memory>

NAMESPACE_BEGIN(util)

struct ConditionalMutex {
  std::unique_ptr<std::recursive_mutex> _mutex;

  ConditionalMutex(bool lock) : _mutex(lock ? std::make_unique<std::recursive_mutex>() : nullptr) {}

  void lock() { if (_mutex) _mutex->lock(); }
  void unlock() { if (_mutex) _mutex->unlock(); }
  bool try_lock() { if (_mutex) return _mutex->try_lock(); return true; }
};

NAMESPACE_END(util)