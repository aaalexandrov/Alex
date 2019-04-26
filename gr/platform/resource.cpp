#include "resource.h"

namespace platform {

void ResourceHolder::RegisterResource(Resource *resource)
{
  ASSERT(_resources.find(resource) == _resources.end());
  ASSERT(resource->_holder == nullptr);
  _resources.insert(resource);
  resource->SetHolder(this);
}

void ResourceHolder::UnregisterResource(Resource *resource)
{
  ASSERT(resource->_holder == this);
  resource->SetHolder(nullptr);
  _resources.erase(resource);
}

void ResourceHolder::DestroyResources()
{
  std::vector<Resource*> resources{ _resources.begin(), _resources.end() };
  for (Resource* r : resources) {
    delete r;
  }
}

}