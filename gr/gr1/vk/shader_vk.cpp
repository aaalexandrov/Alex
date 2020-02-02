#include "shader_vk.h"
#include "device_vk.h"
#include "execution_queue_vk.h"
#include "../graphics_exception.h"
#include "rttr/registration.h"
#include "util/dbg.h"


NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<ShaderVk>("ShaderVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(rttr::policy::ctor::as_raw_ptr);
}

std::vector<ShaderVk::ShaderKindInfo> ShaderVk::_shaderKinds = {
  { ShaderKind::Vertex, vk::ShaderStageFlagBits::eVertex, shaderc_shader_kind::shaderc_vertex_shader },
  { ShaderKind::Fragment, vk::ShaderStageFlagBits::eFragment, shaderc_shader_kind::shaderc_fragment_shader },
};

ShaderVk::ShaderKindInfo *ShaderVk::GetShaderKindInfo(ShaderKind kind)
{
  auto found = std::find_if(_shaderKinds.begin(), _shaderKinds.end(), [=](ShaderKindInfo &info) { return info._kind == kind; });
  return found != _shaderKinds.end() ? &*found : nullptr;
}

vk::ShaderStageFlags ShaderVk::GetShaderStageFlags(ShaderKind kindMask)
{
  vk::ShaderStageFlags stageFlags;
  for (auto &kindInfo : _shaderKinds)
    if ((kindMask & kindInfo._kind) != ShaderKind::None)
      stageFlags |= kindInfo._stage;
  return stageFlags;
}

void ShaderVk::LoadShader(std::vector<uint8_t> const &contents)
{
  LoadModule(contents);

  InitDescriptorSetLayouts();
  InitPipelineLayout();
}

void ShaderVk::LoadModule(std::vector<uint8_t> const &contents)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

	ShaderKindInfo *kind = GetShaderKindInfo(_kind);
  shaderc::SpvCompilationResult res = 
    compiler.CompileGlslToSpv(reinterpret_cast<char const *>(contents.data()), contents.size(), kind->_shadercKind, _name.c_str(), "main", options);
  if (res.GetCompilationStatus() != shaderc_compilation_status_success)
    throw GraphicsException("Shader " + _name + " compilation failed with error : " + res.GetErrorMessage(), VK_RESULT_MAX_ENUM);

	vk::ShaderModuleCreateInfo moduleInfo;
	moduleInfo
		.setCodeSize((res.cend() - res.cbegin()) * sizeof(uint32_t))
		.setPCode(res.cbegin());
  _module = deviceVk->_device->createShaderModuleUnique(moduleInfo, deviceVk->AllocationCallbacks());
	_stageFlags = kind->_stage;

  spirv_cross::Compiler refl(res.cbegin(), res.cend() - res.cbegin());

  if (kind->_stage == vk::ShaderStageFlagBits::eVertex) {
    InitVertexDescription(refl);
  }

  InitUniformBufferDescriptions(refl, _kind);
}

std::vector<vk::PipelineShaderStageCreateInfo> ShaderVk::GetPipelineShaderStageCreateInfos()
{
  std::vector<vk::PipelineShaderStageCreateInfo> infos;
  infos.emplace_back(vk::PipelineShaderStageCreateFlags(), _stageFlags, *_module, "main");
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

static std::unordered_map<SPIRTypeInfo, rttr::type, SPIRTypeInfoHasher> SPIRType2TypeInfo = {
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 1), rttr::type::get<float>()     },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 2), rttr::type::get<glm::vec2>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 3), rttr::type::get<glm::vec3>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 4), rttr::type::get<glm::vec4>() },

  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 2, 2), rttr::type::get<glm::mat2>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 3, 3), rttr::type::get<glm::mat3>() },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 4, 4), rttr::type::get<glm::mat4>() },
};

static std::unordered_map<rttr::type, vk::Format> Type2VkFormat {
  { rttr::type::get<float>()    , vk::Format::eR32Sfloat          },
  { rttr::type::get<glm::vec2>(), vk::Format::eR32G32Sfloat       },
  { rttr::type::get<glm::vec3>(), vk::Format::eR32G32B32Sfloat    },
  { rttr::type::get<glm::vec4>(), vk::Format::eR32G32B32A32Sfloat },
};

rttr::type ShaderVk::GetTypeFromSpirv(spirv_cross::SPIRType type)
{
  rttr::type typeInfo = SPIRType2TypeInfo.at(SPIRTypeInfo(type));
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

vk::DescriptorSetLayoutCreateInfo ShaderVk::GetDescriptorSetLayoutCreateInfos(std::vector<vk::DescriptorSetLayoutBinding> &layoutBindings)
{
  vk::DescriptorSetLayoutCreateInfo layoutInfo;

  layoutInfo
    .setBindingCount(static_cast<uint32_t>(layoutBindings.size()))
    .setPBindings(layoutBindings.data());

  return layoutInfo;
}

std::vector<vk::DescriptorSetLayoutBinding> ShaderVk::GetDescriptorSetLayoutBindings()
{
  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  for (auto &bufferInfo : _uniformBuffers) {
    vk::DescriptorSetLayoutBinding binding;
    binding
      .setBinding(bufferInfo._binding)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setStageFlags(GetShaderStageFlags(_kind));

    bindings.emplace_back(std::move(binding));
  }

  // TODO: add samplers, etc.
  return bindings;
}

void ShaderVk::InitVertexDescription(spirv_cross::Compiler const &reflected)
{
  ASSERT(_vertexDesc->_elements.size() == 0);

  size_t offset = 0;
  for (auto s : reflected.get_shader_resources().stage_inputs) {
    auto &type = reflected.get_type(s.type_id);

    rttr::type typeInfo = GetTypeFromSpirv(type);

    uint32_t location = reflected.get_decoration(s.id, spv::Decoration::DecorationLocation);

    BufferElement elem { typeInfo, offset, location };
    _vertexDesc->AddElement(s.name, std::move(elem));
    offset += typeInfo.get_sizeof();
  }
}

void ShaderVk::InitUniformBufferDescriptions(spirv_cross::Compiler const &reflected, ShaderKind kind)
{
  for (auto u : reflected.get_shader_resources().uniform_buffers) {
    auto &type = reflected.get_type(u.type_id);

    uint32_t binding = reflected.get_decoration(u.id, spv::Decoration::DecorationBinding);
    std::string const &name = reflected.get_name(u.id);
    size_t size = reflected.get_declared_struct_size(type);

    UniformBufferInfo uniformInfo;
    uniformInfo._name = name;
    uniformInfo._binding = binding;

    auto members = type.member_types.size();
    for (auto i = 0; i < members; ++i) {
      auto &memberType = reflected.get_type(type.member_types[i]);
      std::string const &memberName = reflected.get_member_name(type.self, i);
      size_t memberSize = reflected.get_declared_struct_member_size(type, i);
      size_t memberOffset = reflected.type_struct_member_offset(type, i);

      rttr::type elemType = GetTypeFromSpirv(memberType);
      ASSERT(memberSize == elemType.get_sizeof());
      BufferElement elem { elemType, memberOffset };

      uniformInfo._uniformDesc->AddElement(memberName, std::move(elem));
    }

    ASSERT(uniformInfo._uniformDesc->_size <= size);
    uniformInfo._uniformDesc->_size = size;
    _uniformBuffers.push_back(std::move(uniformInfo));
  }
}

void ShaderVk::InitDescriptorSetLayouts()
{
  ASSERT(!_descriptorSetLayouts.size());
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	
  auto layoutBindings = GetDescriptorSetLayoutBindings();
  vk::DescriptorSetLayoutCreateInfo layoutInfo = GetDescriptorSetLayoutCreateInfos(layoutBindings);
  _descriptorSetLayouts.emplace_back(std::move(deviceVk->_device->createDescriptorSetLayoutUnique(layoutInfo, deviceVk->AllocationCallbacks())));
}

void ShaderVk::InitPipelineLayout()
{
  ASSERT(!_pipelineLayout);
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	
  std::vector<vk::DescriptorSetLayout> setLayouts;
  for (auto &layout : _descriptorSetLayouts)
    setLayouts.push_back(layout.get());

  vk::PipelineLayoutCreateInfo pipelineInfo;
  pipelineInfo
    .setSetLayoutCount(static_cast<uint32_t>(setLayouts.size()))
    .setPSetLayouts(setLayouts.data());
  _pipelineLayout = deviceVk->_device->createPipelineLayoutUnique(pipelineInfo, deviceVk->AllocationCallbacks());
}

NAMESPACE_END(gr1)

