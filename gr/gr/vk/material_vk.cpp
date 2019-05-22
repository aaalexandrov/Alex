#include "material_vk.h"
#include "device_vk.h"

NAMESPACE_BEGIN(gr)

MaterialVk::MaterialVk(DeviceVk &device, std::shared_ptr<Shader> &shader) 
  : Material(shader)
  , _device { &device } 
{
}

util::TypeInfo * MaterialVk::GetType()
{
  return util::TypeInfo::Get<MaterialVk>();
}

NAMESPACE_END(gr)