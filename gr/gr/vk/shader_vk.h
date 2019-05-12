#pragma once

#include "util/namespace.h"
#include "../shader.h"
#include "vk.h"

NAMESPACE_BEGIN(gr)

class DeviceVk;
class GraphicsVk;

class ShaderVk : public Shader {
public:
  struct ModuleInfo {
    vk::UniqueShaderModule _module;
    vk::ShaderStageFlagBits _stageFlags;
  };

  ShaderVk(DeviceVk &device, std::string const &name);

  GraphicsVk *GetGraphics();

  void LoadModules();

  std::vector<vk::PipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos();

  std::vector<vk::VertexInputAttributeDescription> GetVertexAttributeDescriptions(spirv_cross::CompilerReflection const &reflected);

  DeviceVk *_device;
  std::vector<ModuleInfo> _modules;

  struct ShaderKind {
    std::string _extention;
    vk::ShaderStageFlagBits _stage;
    shaderc_shader_kind _kind;
  };

  static std::vector<ShaderKind> _shaderKinds;
};

NAMESPACE_END(gr)