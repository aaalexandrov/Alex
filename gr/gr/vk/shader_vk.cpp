#include "shader_vk.h"
#include "graphics_vk.h"
#include "device_vk.h"
#include "util/file.h"
#include "util/dbg.h"


NAMESPACE_BEGIN(gr)

std::vector<ShaderVk::ShaderKind> ShaderVk::_shaderKinds = {
  { ".vert", vk::ShaderStageFlagBits::eVertex, shaderc_shader_kind::shaderc_vertex_shader },
  { ".frag", vk::ShaderStageFlagBits::eFragment, shaderc_shader_kind::shaderc_fragment_shader },
};

ShaderVk::ShaderVk(DeviceVk &device, std::string const &name)
  : Shader { name }
  , _device { &device }
{
  LoadModules();
}

GraphicsVk *ShaderVk::GetGraphics()
{
  return _device->GetGraphics();
}

void ShaderVk::LoadModules()
{
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  for (auto &kind : _shaderKinds) {
    std::string name = _name + kind._extention;
    std::string path = GetGraphics()->GetResourcePath(name);
    std::vector<uint8_t> source = util::ReadFile(path);
    
    shaderc::SpvCompilationResult res = 
      compiler.CompileGlslToSpv(reinterpret_cast<char const *>(source.data()), source.size(), kind._kind, path.c_str(), "main", options);
    if (res.GetCompilationStatus() != shaderc_compilation_status_success)
      throw GraphicsException("Shader " + path + " compilation failed with error : " + res.GetErrorMessage(), VK_RESULT_MAX_ENUM);

    vk::ShaderModuleCreateInfo moduleInfo(vk::ShaderModuleCreateFlags(), res.cend() - res.cbegin(), res.cbegin());
    auto shaderModule = _device->_device->createShaderModuleUnique(moduleInfo, _device->GetGraphics()->AllocationCallbacks());
    _modules.push_back({ std::move(shaderModule), kind._stage });

    if (kind._stage == vk::ShaderStageFlagBits::eVertex) {
      spirv_cross::CompilerReflection refl(res.cbegin(), res.cend() - res.cbegin());
      GetVertexAttributeDescriptions(refl);
    }
  }
}

std::vector<vk::PipelineShaderStageCreateInfo> ShaderVk::GetPipelineShaderStageCreateInfos()
{
  std::vector<vk::PipelineShaderStageCreateInfo> infos;
  for (auto &mod : _modules) {
    infos.emplace_back(vk::PipelineShaderStageCreateFlags(), mod._stageFlags, *mod._module, "main");
  }
  return infos;
}

std::vector<vk::VertexInputAttributeDescription> ShaderVk::GetVertexAttributeDescriptions(spirv_cross::CompilerReflection const &reflected)
{
  std::vector<vk::VertexInputAttributeDescription> descriptions;

  for (auto s : reflected.get_shader_resources().stage_inputs) {
    auto type = reflected.get_type(s.type_id);

    uint32_t location = reflected.get_decoration(s.id, spv::Decoration::DecorationLocation);
    uint32_t alignment = reflected.get_decoration(s.id, spv::Decoration::DecorationAlignment);

    LOG("Attribute location: ", location, " ", s.name, " : ", type.basetype, "[", type.vecsize, "] width: ", type.width, ", columns: ", type.columns, ", align: ", alignment);
  }

  for (auto u : reflected.get_shader_resources().uniform_buffers) {
    auto type = reflected.get_type(u.type_id);

    auto binding = reflected.get_decoration(u.id, spv::Decoration::DecorationBinding);
    auto name = reflected.get_name(u.id);
    auto size = reflected.get_declared_struct_size(type);

    LOG("Uniform binding: ", binding, " ", name, " size: ", size);

    auto members = type.member_types.size();
    for (auto i = 0; i < members; ++i) {
      auto &memberType = reflected.get_type(type.member_types[i]);
      auto memberName = reflected.get_member_name(type.self, i);
      size_t memberSize = reflected.get_declared_struct_member_size(type, i);
      size_t memberOffset = reflected.type_struct_member_offset(type, i);

      LOG("  Member ", memberName, " : ", memberType.basetype, " offset: ", memberOffset, " size: ", memberSize);
    }
  }

  return descriptions;
}



NAMESPACE_END(gr)

