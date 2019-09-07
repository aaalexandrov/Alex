#pragma once

#include "util/namespace.h"
#include "util/rtti.h"

NAMESPACE_BEGIN(gr)

class ResourceUpdate;

class GraphicsResource : public std::enable_shared_from_this<GraphicsResource> {
public:
  virtual util::TypeInfo *GetType() = 0;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::GraphicsResource)
