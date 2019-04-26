#pragma once

namespace gr {

class PresentationSurface {
public:
  struct CreateData {

  };

  PresentationSurface(CreateData &createData);
  virtual ~PresentationSurface();
};

}