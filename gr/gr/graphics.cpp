#include "graphics.h"
#include "stb/stb_image.h"
#include "util/mem.h"

#ifdef GR_VK
#include "vk/graphics_vk.h"
#endif

NAMESPACE_BEGIN(gr)

Graphics *Graphics::Create()
{
#ifdef GR_VK
  return new GraphicsVk();
#endif
}

Graphics::Graphics()
{
#ifdef _DEBUG
  _validationLevel = ValidationLevel::Full;
#endif
  _indexDescU16->AddElement("index", { util::TypeInfo::Get<uint16_t>(), 0 });
  _indexDescU32->AddElement("index", { util::TypeInfo::Get<uint32_t>(), 0 });
  _rawDescU8->AddElement("data", { util::TypeInfo::Get<uint8_t>(), 0 });
}

Graphics::~Graphics()
{
}

std::shared_ptr<Image> Graphics::LoadImage(std::string const &name)
{
  std::string path = GetResourcePath(name);
  int width, height, channels;

  util::AutoFree<stbi_uc*> imgPixels { stbi_load(path.c_str(), &width, &height, &channels, 4), stbi_image_free };

  auto img = CreateImage(Image::Usage::Texture, ColorFormat::R8G8B8A8, glm::uvec4(width, height, 0, 0), 1);
  util::BoxWithLayer region { glm::zero<glm::uvec4>(), img->GetSize() - glm::one<glm::uvec4>() };
  ImageData imgData { img->GetSize(), ImageData::GetPackedPitch(img->GetSize(), 4), imgPixels.Get() };
  img->UpdateContents(region, 0, imgData, glm::zero<glm::uvec4>());

  return img;
}

void Graphics::SetLoadPath(std::string loadPath)
{
  _loadPath = loadPath;
  if (_loadPath.length() > 0 && _loadPath[_loadPath.length() - 1] != '/')
    _loadPath += '/';
}

std::string Graphics::GetResourcePath(std::string name)
{
  return _loadPath + name;
}

NAMESPACE_END(gr)