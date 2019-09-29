#pragma once

#include "../material.h"
#include "graphics_state_vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

class MaterialVk : public Material {
public:
  MaterialVk(DeviceVk &device, std::shared_ptr<Shader> &shader);

  util::TypeInfo *GetType() override;

  GraphicsStateVk *GetState() override { return &_state; }

  DeviceVk *_device;
  GraphicsStateVk _state;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::MaterialVk, gr::Material)