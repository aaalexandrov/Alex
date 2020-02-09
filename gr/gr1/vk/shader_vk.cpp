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

vk::ShaderStageFlags ShaderVk::GetShaderStageFlags(ShaderKind kind)
{
  vk::ShaderStageFlags stageFlags = GetShaderKindInfo(kind)->_stage;
  return stageFlags;
}

void ShaderVk::LoadShader(std::vector<uint8_t> const &contents)
{
  LoadModule(contents);

  InitDescriptorSetLayoutAndStore();

	_state = ResourceState::ShaderRead;
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

  InitUniformBufferDescriptions(refl);
}

vk::PipelineShaderStageCreateInfo ShaderVk::GetPipelineShaderStageCreateInfo()
{
	vk::PipelineShaderStageCreateInfo info;
	info
		.setStage(_stageFlags)
		.setModule(*_module)
		.setPName("main");
	return info;
}

vk::UniqueDescriptorSet ShaderVk::AllocateDescriptorSet()
{
	if (!_descSetStore.IsValid())
		return vk::UniqueDescriptorSet();
	return _descSetStore.AllocateDescriptorSet(*_descriptorSetLayout);
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

std::unordered_map<rttr::type, vk::Format> Type2VkFormat {
  { rttr::type::get<float>()    , vk::Format::eR32Sfloat          },
  { rttr::type::get<glm::vec2>(), vk::Format::eR32G32Sfloat       },
  { rttr::type::get<glm::vec3>(), vk::Format::eR32G32B32Sfloat    },
  { rttr::type::get<glm::vec4>(), vk::Format::eR32G32B32A32Sfloat },
};

rttr::type ShaderVk::GetTypeFromSpirv(spirv_cross::SPIRType const &type)
{
  rttr::type typeInfo = SPIRType2TypeInfo.at(SPIRTypeInfo(type));
  return typeInfo;
}

std::shared_ptr<util::LayoutElement> ShaderVk::GetLayoutFromSpirv(spirv_cross::Compiler const &reflected, spirv_cross::SPIRType const &type, size_t typeSize)
{
	size_t arrayElements = 1;
	for (auto dim : type.array) {
		arrayElements *= dim;
	}

	std::shared_ptr<util::LayoutElement> elem;
	if (type.basetype == spirv_cross::SPIRType::Struct) {
		std::vector<std::pair<std::shared_ptr<util::LayoutElement>, std::string>> members;
		size_t structSize = reflected.get_declared_struct_size(type);
		for (uint32_t i = 0; i < type.member_types.size(); ++i) {
			auto &memberType = reflected.get_type(type.member_types[i]);
			auto &memberName = reflected.get_member_name(type.self, i);
			size_t memberSize = reflected.get_declared_struct_member_size(type, i);
			size_t memberOffset = reflected.type_struct_member_offset(type, i);
			size_t effectiveSize = i < type.member_types.size() - 1 ? reflected.type_struct_member_offset(type, i + 1) : structSize;
			effectiveSize -= memberOffset;
			ASSERT(effectiveSize == memberSize);
			std::shared_ptr<util::LayoutElement> memberLayout = GetLayoutFromSpirv(reflected, memberType, memberSize);
			members.emplace_back(std::move(memberLayout), memberName);
		}
		ASSERT(typeSize == structSize * arrayElements);
		elem = std::make_shared<util::LayoutStruct>(members, structSize);
	} else {
		elem = std::make_shared<util::LayoutValue>(GetTypeFromSpirv(type), typeSize / arrayElements);
	}

	for (size_t i = 0; i < type.array.size(); ++i) {
		ASSERT(type.array_size_literal[i] && "Arrays with specialization constants not supported");
		std::shared_ptr<util::LayoutElement> arr = std::make_shared<util::LayoutArray>(elem, type.array[i], typeSize);
		elem = std::move(arr);
	}

	return elem;
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
  ASSERT(!_vertexLayout);

	std::vector<std::pair<std::shared_ptr<util::LayoutElement>, std::string>> attributes;
  for (auto s : reflected.get_shader_resources().stage_inputs) {
    auto &type = reflected.get_type(s.type_id);

		std::shared_ptr<util::LayoutElement> attrLayout = GetLayoutFromSpirv(reflected, type);
		attributes.emplace_back(attrLayout, s.name);
  }

	_vertexLayout = std::make_shared<util::LayoutStruct>(attributes);

	for (uint32_t i = 0; i < reflected.get_shader_resources().stage_inputs.size(); ++i) {
		auto &s = reflected.get_shader_resources().stage_inputs[i];
		auto &type = reflected.get_type(s.type_id);

		uint32_t location = reflected.get_decoration(s.id, spv::Decoration::DecorationLocation);
		_vertexLayout->GetStructFieldElement(i)->SetUserData(location);
	}
}

void ShaderVk::InitUniformBufferDescriptions(spirv_cross::Compiler const &reflected)
{
  for (auto u : reflected.get_shader_resources().uniform_buffers) {
    auto &type = reflected.get_type(u.type_id);

    uint32_t binding = reflected.get_decoration(u.id, spv::Decoration::DecorationBinding);
    std::string const &name = reflected.get_name(u.id);
    size_t size = reflected.get_declared_struct_size(type);

    UniformBufferInfo uniformInfo;
    uniformInfo._name = name;
    uniformInfo._binding = binding;
		uniformInfo._layout = GetLayoutFromSpirv(reflected, type, size);
    _uniformBuffers.push_back(std::move(uniformInfo));
  }
}

void ShaderVk::InitDescriptorSetLayoutAndStore()
{
  ASSERT(!_descriptorSetLayout);
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	
	auto layoutBindings = GetDescriptorSetLayoutBindings();
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo
		.setBindingCount(static_cast<uint32_t>(layoutBindings.size()))
		.setPBindings(layoutBindings.data());
  _descriptorSetLayout = deviceVk->_device->createDescriptorSetLayoutUnique(layoutInfo, deviceVk->AllocationCallbacks());

	if (layoutBindings.size()) {
		uint32_t const descriptorsPerPool = 256;
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (auto &binding : layoutBindings) {
			auto it = std::find_if(poolSizes.begin(), poolSizes.end(), [&](auto &poolSize) {
				return poolSize.type == binding.descriptorType;
			});
			if (it == poolSizes.end()) {
				poolSizes.emplace_back(binding.descriptorType);
				it = poolSizes.end() - 1;
			}
			it->setDescriptorCount(it->descriptorCount + descriptorsPerPool);
		}
		_descSetStore.Init(*deviceVk, poolSizes, descriptorsPerPool);
	}
}

NAMESPACE_END(gr1)

