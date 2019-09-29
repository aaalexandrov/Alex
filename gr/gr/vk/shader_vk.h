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

  util::TypeInfo *GetType() override;

  GraphicsVk *GetGraphics();

  void LoadModules();

  std::vector<vk::PipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos();
  vk::PipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfos(
    std::vector<vk::VertexInputAttributeDescription> &vertexAttribs, 
    std::vector<vk::VertexInputBindingDescription> &vertexBinds);

  std::vector<vk::VertexInputAttributeDescription> GetVertexAttributeDescriptions();
  std::vector<vk::VertexInputBindingDescription> GetVertexBindingDescriptions();

  vk::DescriptorSetLayoutCreateInfo GetDescriptorSetLayoutCreateInfos(std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings);
  std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings();

  static util::TypeInfo *GetTypeInfoFromSpirv(spirv_cross::SPIRType type);
  void InitVertexDescription(spirv_cross::Compiler const &reflected);
  void InitUniformBufferDescriptions(spirv_cross::Compiler const &reflected, ShaderKind kind);
  void InitDescriptorSetLayouts();
  void InitPipelineLayout();


  DeviceVk *_device;
  std::vector<ModuleInfo> _modules;
  std::vector<vk::UniqueDescriptorSetLayout> _descriptorSetLayouts;
  vk::UniquePipelineLayout _pipelineLayout;

  struct ShaderKindInfo {
    ShaderKind _kind;
    std::string _extention;
    vk::ShaderStageFlagBits _stage;
    shaderc_shader_kind _shadercKind;
  };

  static ShaderKindInfo *GetShaderKindInfo(ShaderKind kind);
  static vk::ShaderStageFlags GetShaderStageFlags(ShaderKind kindMask);

  static std::vector<ShaderKindInfo> _shaderKinds;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::ShaderVk, gr::Shader)