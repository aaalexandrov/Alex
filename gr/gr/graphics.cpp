#include "graphics.h"

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