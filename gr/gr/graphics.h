#pragma once

#include <string>
#include "util/namespace.h"
#include "graphics_exception.h"
#include "image.h"
#include "buffer.h"

NAMESPACE_BEGIN(gr)

struct Version {
  uint32_t _major, _minor, _patch;
};

class PresentationSurface;
struct PresentationSurfaceCreateData;
class Shader;

class Graphics {
public:
  enum class ValidationLevel {
    None,
    Full,
  };

  static Graphics *Create();

  Graphics();
  virtual ~Graphics();

  virtual void Init(PresentationSurfaceCreateData &surfaceData) = 0;

  virtual PresentationSurface *CreatePresentationSurface(PresentationSurfaceCreateData &createData) = 0;
  virtual PresentationSurface *GetDefaultPresentationSurface() = 0;

  virtual Buffer *CreateBuffer(Buffer::Usage usage, BufferDescPtr &description, size_t size) = 0;
  virtual Image *CreateImage(Image::Usage usage, ColorFormat format, glm::u32vec3 size, uint32_t mipLevels, uint32_t arrayLayers) = 0;

  virtual Shader *LoadShader(std::string const &name) = 0;

  virtual void SetLoadPath(std::string loadPath);
  virtual std::string GetResourcePath(std::string name);

  ValidationLevel _validationLevel = ValidationLevel::None;
  std::string _appName { "gr app" };
  Version _appVersion { 0, 0, 0 };
  std::string _loadPath;
};

NAMESPACE_END(gr)