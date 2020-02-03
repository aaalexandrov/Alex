#pragma once

#include "../shader.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class ShaderVk : public Shader {
	RTTR_ENABLE(Shader)
public:
  ShaderVk(Device &device) : Shader(device) {}

	void LoadShader(std::vector<uint8_t> const &contents) override;

protected:
	void LoadModule(std::vector<uint8_t> const &contents);

  std::vector<vk::PipelineShaderStageCreateInfo> GetPipelineShaderStageCreateInfos();
  vk::PipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfos(
    std::vector<vk::VertexInputAttributeDescription> &vertexAttribs, 
    std::vector<vk::VertexInputBindingDescription> &vertexBinds);

  std::vector<vk::VertexInputAttributeDescription> GetVertexAttributeDescriptions();
  std::vector<vk::VertexInputBindingDescription> GetVertexBindingDescriptions();

  vk::DescriptorSetLayoutCreateInfo GetDescriptorSetLayoutCreateInfos(std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings);
  std::vector<vk::DescriptorSetLayoutBinding> GetDescriptorSetLayoutBindings();

  static rttr::type GetTypeFromSpirv(spirv_cross::SPIRType const &type);
	static std::shared_ptr<util::LayoutElement> GetLayoutFromSpirv(spirv_cross::Compiler const &reflected, spirv_cross::SPIRType const &type, size_t typeSize = 0);
  void InitVertexDescription(spirv_cross::Compiler const &reflected);
  void InitUniformBufferDescriptions(spirv_cross::Compiler const &reflected);
  void InitDescriptorSetLayouts();
  void InitPipelineLayout();


	vk::UniqueShaderModule _module;
	vk::ShaderStageFlagBits _stageFlags;
  std::vector<vk::UniqueDescriptorSetLayout> _descriptorSetLayouts;
  vk::UniquePipelineLayout _pipelineLayout;

  struct ShaderKindInfo {
    ShaderKind _kind;
    vk::ShaderStageFlagBits _stage;
    shaderc_shader_kind _shadercKind;
  };

  static ShaderKindInfo *GetShaderKindInfo(ShaderKind kind);
  static vk::ShaderStageFlags GetShaderStageFlags(ShaderKind kindMask);

  static std::vector<ShaderKindInfo> _shaderKinds;
};

NAMESPACE_END(gr1)
