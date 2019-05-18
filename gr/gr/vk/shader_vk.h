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
  vk::PipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfos(
    std::vector<vk::VertexInputAttributeDescription> &vertexAttribs, 
    std::vector<vk::VertexInputBindingDescription> &vertexBinds);

  std::vector<vk::VertexInputAttributeDescription> GetVertexAttributeDescriptions();
  std::vector<vk::VertexInputBindingDescription> GetVertexBindingDescriptions();

  static util::TypeInfo *GetTypeInfoFromSpirv(spirv_cross::SPIRType type);
  void InitVertexDescription(spirv_cross::Compiler const &reflected);
  void InitUniformBufferDescriptions(spirv_cross::Compiler const &reflected);


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