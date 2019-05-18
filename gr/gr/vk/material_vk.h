#pragma once

#include "../material.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;

class MaterialVk : public Material {
public:
  MaterialVk(DeviceVk &device, std::shared_ptr<Shader> &shader);

  DeviceVk *_device;
};

NAMESPACE_END(gr)