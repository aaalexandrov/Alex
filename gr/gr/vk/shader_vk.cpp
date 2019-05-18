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
    auto shaderModule = _device->_device->createShaderModuleUnique(moduleInfo, _device->AllocationCallbacks());
    _modules.push_back({ std::move(shaderModule), kind._stage });

    spirv_cross::Compiler refl(res.cbegin(), res.cend() - res.cbegin());

    if (kind._stage == vk::ShaderStageFlagBits::eVertex) {
      InitVertexDescription(refl);
    }

    InitUniformBufferDescriptions(refl);
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

vk::PipelineVertexInputStateCreateInfo ShaderVk::GetPipelineVertexInputStateCreateInfos(
  std::vector<vk::VertexInputAttributeDescription> &vertexAttribs,
  std::vector<vk::VertexInputBindingDescription> &vertexBinds)
{
  vertexAttribs = GetVertexAttributeDescriptions();
  vertexBinds = GetVertexBindingDescriptions();
  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo
    .setVertexBindingDescriptionCount(static_cast<uint32_t>(vertexBinds.size()))
    .setPVertexBindingDescriptions(vertexBinds.data())
    .setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexAttribs.size()))
    .setPVertexAttributeDescriptions(vertexAttribs.data());
  return vertexInputInfo;
}

struct SPIRTypeInfo {
  spirv_cross::SPIRType::BaseType _baseType;
  uint32_t _columns, _vecSize;

  SPIRTypeInfo(spirv_cross::SPIRType::BaseType baseType, uint32_t columns, uint32_t vecSize)
    : _baseType(baseType)
    , _columns(columns)
    , _vecSize(vecSize)
  {}

  SPIRTypeInfo(spirv_cross::SPIRType const &type)
    : _baseType(type.basetype)
    , _columns(type.columns)
    , _vecSize(type.vecsize)
  {}

  static size_t hash(SPIRTypeInfo const &ti)
  {
    size_t h = 17;
    h = h * 31 + std::hash<int>()(ti._baseType);
    h = h * 31 + std::hash<uint32_t>()(ti._columns);
    h = h * 31 + std::hash<uint32_t>()(ti._vecSize);
    return h;
  }

  bool operator ==(SPIRTypeInfo const &other) const
  {
    return _baseType == other._baseType
      && _columns == other._columns
      && _vecSize == other._vecSize;
  }
};

struct SPIRTypeInfoHasher {
  size_t operator()(SPIRTypeInfo const &ti) const { return SPIRTypeInfo::hash(ti); }
};

static std::unordered_map<SPIRTypeInfo, util::TypeInfo*, SPIRTypeInfoHasher> SPIRType2TypeInfo = {
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 1), util::TypeInfo::Get<float>()     },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 2), util::TypeInfo::Get<glm::vec2>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 3), util::TypeInfo::Get<glm::vec3>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 4), util::TypeInfo::Get<glm::vec4>() },

  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 2, 2), util::TypeInfo::Get<glm::mat2>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 3, 3), util::TypeInfo::Get<glm::mat3>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 4, 4), util::TypeInfo::Get<glm::mat4>() },
};

static std::unordered_map<util::TypeInfo*, vk::Format> Type2VkFormat {
  { util::TypeInfo::Get<float>()    , vk::Format::eR32Sfloat          },
  { util::TypeInfo::Get<glm::vec2>(), vk::Format::eR32G32Sfloat       },
  { util::TypeInfo::Get<glm::vec3>(), vk::Format::eR32G32B32Sfloat    },
  { util::TypeInfo::Get<glm::vec4>(), vk::Format::eR32G32B32A32Sfloat },
};

util::TypeInfo *ShaderVk::GetTypeInfoFromSpirv(spirv_cross::SPIRType type)
{
  util::TypeInfo *typeInfo = SPIRType2TypeInfo.at(SPIRTypeInfo(type));
  return typeInfo;
}

std::vector<vk::VertexInputAttributeDescription> ShaderVk::GetVertexAttributeDescriptions()
{
  std::vector<vk::VertexInputAttributeDescription> descriptions;
  uint32_t binding = 0;

  for (auto &nameElem : _vertexDesc->_elements) {
    descriptions.emplace_back(
      nameElem.second._location, 
      binding,
      Type2VkFormat.at(nameElem.second._type), 
      static_cast<uint32_t>(nameElem.second._offset));
  }

  std::sort(descriptions.begin(), descriptions.end(), 
    [](vk::VertexInputAttributeDescription const &d1, vk::VertexInputAttributeDescription const &d2) { return d1.offset < d2.offset; });

  return descriptions;
}

std::vector<vk::VertexInputBindingDescription> ShaderVk::GetVertexBindingDescriptions()
{
  std::vector<vk::VertexInputBindingDescription> descriptions;
  uint32_t binding = 0;

  descriptions.emplace_back(binding, static_cast<uint32_t>(_vertexDesc->_size), vk::VertexInputRate::eVertex);

  return descriptions;
}

void ShaderVk::InitVertexDescription(spirv_cross::Compiler const &reflected)
{
  ASSERT(_vertexDesc->_elements.size() == 0);

  size_t offset = 0;
  for (auto s : reflected.get_shader_resources().stage_inputs) {
    auto &type = reflected.get_type(s.type_id);

    util::TypeInfo *typeInfo = GetTypeInfoFromSpirv(type);

    uint32_t location = reflected.get_decoration(s.id, spv::Decoration::DecorationLocation);

    BufferElement elem { typeInfo, offset, location };
    _vertexDesc->AddElement(s.name, std::move(elem));
    offset += typeInfo->GetSize();
  }
}

void ShaderVk::InitUniformBufferDescriptions(spirv_cross::Compiler const &reflected)
{
  for (auto u : reflected.get_shader_resources().uniform_buffers) {
    auto &type = reflected.get_type(u.type_id);

    uint32_t binding = reflected.get_decoration(u.id, spv::Decoration::DecorationBinding);
    std::string const &name = reflected.get_name(u.id);
    size_t size = reflected.get_declared_struct_size(type);

    auto found = std::find_if(_uniformBuffers.begin(), _uniformBuffers.end(), 
      [&name](UniformBufferInfo const &uniformInfo) { return uniformInfo._name == name; });
    if (found != _uniformBuffers.end()) {
      // uniform buffers with the same name in different shader kinds need to be the same
      ASSERT(found->_uniformDesc->_size == size);
      continue;
    }

    UniformBufferInfo uniformInfo;
    uniformInfo._name = name;
    uniformInfo._binding = binding;

    auto members = type.member_types.size();
    for (auto i = 0; i < members; ++i) {
      auto &memberType = reflected.get_type(type.member_types[i]);
      std::string const &memberName = reflected.get_member_name(type.self, i);
      size_t memberSize = reflected.get_declared_struct_member_size(type, i);
      size_t memberOffset = reflected.type_struct_member_offset(type, i);

      util::TypeInfo *elemType = GetTypeInfoFromSpirv(memberType);
      ASSERT(memberSize == elemType->GetSize());
      BufferElement elem { elemType, memberOffset };

      uniformInfo._uniformDesc->AddElement(memberName, std::move(elem));
    }

    ASSERT(uniformInfo._uniformDesc->_size <= size);
    uniformInfo._uniformDesc->_size = size;
    _uniformBuffers.push_back(std::move(uniformInfo));
  }
}


NAMESPACE_END(gr)

