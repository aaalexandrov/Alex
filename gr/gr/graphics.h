#pragma once

#include <string>
#include <memory>
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
class Material;
class Model;
class ModelInstance;
class OperationQueue;

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

  virtual std::shared_ptr<PresentationSurface> CreatePresentationSurface(PresentationSurfaceCreateData &createData) = 0;

  virtual std::shared_ptr<Buffer> CreateBuffer(Buffer::Usage usage, BufferDescPtr &description, size_t size) = 0;
  virtual std::shared_ptr<Image> CreateImage(Image::Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels) = 0;
  virtual std::shared_ptr<Material> CreateMaterial(std::shared_ptr<Shader> &shader) = 0;

  template <typename BufferType>
  std::shared_ptr<BufferType> CreateBufferTyped(Buffer::Usage usage, BufferDescPtr &description, size_t size) { return std::static_pointer_cast<BufferType>(CreateBuffer(usage, description, size)); }
  template <typename ImageType>
  std::shared_ptr<ImageType> CreateImageTyped(Image::Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels) { return std::static_pointer_cast<ImageType>(CreateImage(usage, format, size, mipLevels)); }

  virtual std::shared_ptr<Shader> LoadShader(std::string const &name) = 0;
  std::shared_ptr<Image> LoadImage(std::string const &name);

  virtual void SetLoadPath(std::string loadPath);
  virtual std::string GetResourcePath(std::string name);

  virtual OperationQueue *GetOperationQueue() = 0;

  virtual void Update();

  BufferDescPtr &GetRawBufferDesc()      { return _rawDescU8; }
  BufferDescPtr &GetIndexBufferDescU16() { return _indexDescU16; }
  BufferDescPtr &GetIndexBufferDescU32() { return _indexDescU32; }

  ValidationLevel _validationLevel = ValidationLevel::None;
  std::string _appName { "gr app" };
  Version _appVersion { 0, 0, 0 };
  std::string _loadPath;
  BufferDescPtr _indexDescU16 = BufferDesc::Create(), 
    _indexDescU32 = BufferDesc::Create(), 
    _rawDescU8 = BufferDesc::Create();
};

NAMESPACE_END(gr)