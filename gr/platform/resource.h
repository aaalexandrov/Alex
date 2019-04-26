#pragma once

#include <memory>
#include <unordered_set>

namespace platform {

class Resource;

class ResourceHolder {
public:
  std::unordered_set<Resource*> _resources;

  virtual void RegisterResource(Resource *resource);
  virtual void UnregisterResource(Resource *resource);

  void DestroyResources();
};

class Resource {
public:
  ResourceHolder *_holder = nullptr;

  virtual ~Resource()
  {
    if (_holder)
      _holder->UnregisterResource(this);
  }

  void SetHolder(ResourceHolder *holder) { _holder = holder; }
};

}