#pragma once

#include <utility>
#include <functional>

template <typename R, typename D = std::function<void(R&)>>
struct UniqueResource {
  typedef R Type;
  typedef D Deleter;

  R m_resource;
  D m_deleter;
  bool m_execute;

  UniqueResource(R &&resource, D &&deleter, bool execute = true) noexcept :
    m_resource(std::move(resource)), 
    m_deleter(std::move(deleter)), 
    m_execute{ execute }
  {
  }

  UniqueResource(UniqueResource &&other) noexcept :
    m_resource(std::move(other.m_resource)),
    m_deleter(std::move(other.m_deleter)),
    execute{ other.m_execute }
  {
    other.Release();
  }

  ~UniqueResource() noexcept
  {
    Reset();
  }

  UniqueResource &operator=(UniqueResource &&other) noexcept
  {
    Reset();
    m_resource = std::move(other.m_resource);
    m_deleter = std::move(other.m_deleter);
    m_execute = other.m_execute;
    other.Release();
  }

  R const &Get() const noexcept { return m_resource; }
  D const &GetDeleter() const noexcept { return m_deleter; }

  UniqueResource(UniqueResource const &) = delete;
  UniqueResource &operator=(UniqueResource const &) = delete;

  void Reset() noexcept
  {
    if (m_execute) {
      m_execute = false;
      m_deleter(m_resource);
    }
  }

  void Reset(R &&resource) noexcept
  {
    Reset();
    m_resource = std::move(resource);
    m_execute = true;
  }

  R const &Release() noexcept
  {
    m_execute = false;
    return m_resource;
  }
};

struct OnExit {
  std::function<void()> m_func;
  OnExit(std::function<void()> &&func = nullptr) noexcept : m_func(std::move(func)) {}
  ~OnExit() { Execute(); }

  void Execute() noexcept {
    if (m_func) {
      m_func();
      m_func = nullptr;
    }
  }

  void Reset(std::function<void()> &&func = nullptr) noexcept { m_func = func; }
};