#pragma once

#include "util/namespace.h"

NAMESPACE_BEGIN(gr)

struct PresentationSurfaceCreateData {
};


class PresentationSurface {
public:

  PresentationSurface(PresentationSurfaceCreateData &createData);
  virtual ~PresentationSurface();

  virtual void Update(uint32_t width, uint32_t height) = 0;
};

NAMESPACE_END(gr)