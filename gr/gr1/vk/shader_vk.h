#pragma once

#include "../shader.h"
#include "descriptor_set_store.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class ShaderVk : public Shader {
	RTTR_ENABLE(Shader)
public:
  ShaderVk(Device &device) : Shader(device) {}

	void LoadShader(std::string name, ShaderOptions const &shaderOptions) override;

	vk::PipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo();
	void FillDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding> &bindings);

protected:
	void LoadModule(std::vector<uint8_t> const &contents, ShaderOptions const &shaderOptions);
	shaderc::CompileOptions GetShadercCompileOptions(ShaderOptions const &shaderOptions);

  static rttr::type GetTypeFromSpirv(spirv_cross::SPIRType const &type);
	static std::shared_ptr<util::LayoutElement> GetLayoutFromSpirv(spirv_cross::Compiler const &reflected, spirv_cross::SPIRType const &type, size_t typeSize = 0);
  void InitVertexDescription(spirv_cross::Compiler const &reflected);
  std::vector<Parameter> GetUniformDescriptions(spirv_cross::Compiler const &reflected, spirv_cross::SmallVector<spirv_cross::Resource> const &resources);

	vk::UniqueShaderModule _module;
	vk::ShaderStageFlagBits _stageFlags;

  struct ShaderKindInfo {
    ShaderKind::Enum _kind;
    vk::ShaderStageFlagBits _stage;
    shaderc_shader_kind _shadercKind;
  };

  static ShaderKindInfo *GetShaderKindInfo(ShaderKind::Enum kind);
  static vk::ShaderStageFlags GetShaderStageFlags(ShaderKind::Enum kindMask);

  static std::vector<ShaderKindInfo> _shaderKinds;
};

extern std::unordered_map<rttr::type, vk::Format> Type2VkFormat;

NAMESPACE_END(gr1)
