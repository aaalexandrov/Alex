#pragma once

#include "util/namespace.h"
#include "util/rtti.h"

NAMESPACE_BEGIN(gr)

class ResourceUpdate;

class GraphicsResource : public std::enable_shared_from_this<GraphicsResource> {
public:
  virtual util::TypeInfo *GetType() = 0;

  std::weak_ptr<ResourceUpdate> _lastUpdate;
};

class ResourceUpdate : public GraphicsResource {
public:
  ResourceUpdate(GraphicsResource &updatedResource);

  virtual bool IsComplete() = 0;

  std::shared_ptr<GraphicsResource> _resource;
  std::weak_ptr<ResourceUpdate> _prevUpdate;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::GraphicsResource)
RTTI_BIND(gr::ResourceUpdate, gr::GraphicsResource)