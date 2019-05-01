#pragma once

#include <string>
#include "graphics_exception.h"

namespace gr {

struct Version {
  uint32_t _major, _minor, _patch;
};

class PresentationSurface;
struct PresentationSurfaceCreateData;

class Graphics {
public:
  enum class ValidationLevel {
    None,
    Full,
  };

  ValidationLevel _validationLevel = ValidationLevel::None;
  std::string _appName{ "gr app" };
  Version _appVersion{ 0, 0, 0 };

  static Graphics *Create();

  Graphics();
  virtual ~Graphics();

  virtual void Init(PresentationSurfaceCreateData &surfaceData) = 0;

  virtual PresentationSurface *CreatePresentationSurface(PresentationSurfaceCreateData &createData) = 0;

  virtual PresentationSurface *GetDefaultPresentationSurface() = 0;
};

}