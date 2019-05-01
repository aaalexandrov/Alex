#pragma once

namespace gr {

struct PresentationSurfaceCreateData {
};


class PresentationSurface {
public:

  PresentationSurface(PresentationSurfaceCreateData &createData);
  virtual ~PresentationSurface();
};

}