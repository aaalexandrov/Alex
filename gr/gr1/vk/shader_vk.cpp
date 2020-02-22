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

	_state = ResourceState::ShaderRead;
}

void ShaderVk::LoadModule(std::vector<uint8_t> const &contents)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

	ShaderKindInfo *kind = GetShaderKindInfo(_kind);
  shaderc::SpvCompilationResult res = 
    compiler.CompileGlslToSpv(reinterpret_cast<char const *>(contents.data()), contents.size(), kind->_shadercKind, _name.c_str(), _entryPoint.c_str(), options);
	if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::string message = res.GetErrorMessage();
		throw GraphicsException("Shader " + _name + " compilation failed with error : " + message, VK_RESULT_MAX_ENUM);
	}

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

	_uniformBuffers = GetUniformDescriptions(refl, refl.get_shader_resources().uniform_buffers);
	_samplers = GetUniformDescriptions(refl, refl.get_shader_resources().sampled_images);
}

vk::PipelineShaderStageCreateInfo ShaderVk::GetPipelineShaderStageCreateInfo()
{
	vk::PipelineShaderStageCreateInfo info;
	info
		.setStage(_stageFlags)
		.setModule(*_module)
		.setPName(_entryPoint.c_str());
	return info;
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
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 1), rttr::type::get<float>()       },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 2), rttr::type::get<glm::vec2>()   },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 3), rttr::type::get<glm::vec3>()   },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 1, 4), rttr::type::get<glm::vec4>()   },

	{ SPIRTypeInfo(spirv_cross::SPIRType::UByte, 1, 1), rttr::type::get<uint8_t>()     },
	{ SPIRTypeInfo(spirv_cross::SPIRType::UByte, 1, 2), rttr::type::get<glm::u8vec2>() },
	{ SPIRTypeInfo(spirv_cross::SPIRType::UByte, 1, 3), rttr::type::get<glm::u8vec3>() },
	{ SPIRTypeInfo(spirv_cross::SPIRType::UByte, 1, 4), rttr::type::get<glm::u8vec4>() },

  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 2, 2), rttr::type::get<glm::mat2>()   },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 3, 3), rttr::type::get<glm::mat3>()   },
  { SPIRTypeInfo(spirv_cross::SPIRType::Float, 4, 4), rttr::type::get<glm::mat4>()   },
};

std::unordered_map<rttr::type, vk::Format> Type2VkFormat {
  { rttr::type::get<float>()      , vk::Format::eR32Sfloat          },
  { rttr::type::get<glm::vec2>()  , vk::Format::eR32G32Sfloat       },
  { rttr::type::get<glm::vec3>()  , vk::Format::eR32G32B32Sfloat    },
  { rttr::type::get<glm::vec4>()  , vk::Format::eR32G32B32A32Sfloat },

	{ rttr::type::get<uint8_t>()    , vk::Format::eR8Unorm            },
	{ rttr::type::get<glm::u8vec2>(), vk::Format::eR8G8Unorm          },
	{ rttr::type::get<glm::u8vec3>(), vk::Format::eR8G8B8Unorm        },
	{ rttr::type::get<glm::u8vec4>(), vk::Format::eR8G8B8A8Unorm      },
};

static std::unordered_map<spv::Dim, rttr::type> s_spirvDim2Type{ {
	{ spv::Dim::Dim1D  , rttr::type::get<Texture1D>()   },
	{ spv::Dim::Dim2D  , rttr::type::get<Texture2D>()   },
	{ spv::Dim::Dim3D  , rttr::type::get<Texture3D>()   },
	{ spv::Dim::DimCube, rttr::type::get<TextureCube>() },
} };

rttr::type ShaderVk::GetTypeFromSpirv(spirv_cross::SPIRType const &type)
{
	rttr::type typeInfo = rttr::type::get<void>();
	if (type.basetype == spirv_cross::SPIRType::SampledImage) {
		ASSERT(!type.image.arrayed && "Sampler arrays not currently supported");
		typeInfo = s_spirvDim2Type.at(type.image.dim);
	} else {
		typeInfo = SPIRType2TypeInfo.at(SPIRTypeInfo(type));
	}
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

static void AddSetLayoutBinding(std::vector<vk::DescriptorSetLayoutBinding> &bindings, vk::DescriptorSetLayoutBinding const &binding)
{
	auto it = std::find_if(bindings.begin(), bindings.end(), [&](auto &b) { return b.binding == binding.binding; });
	if (it == bindings.end()) {
		bindings.push_back(binding);
		return;
	}
	if (binding.descriptorType != it->descriptorType || binding.descriptorCount != it->descriptorCount)
		throw GraphicsException("Shaders have the same binding with different parameters!", VK_RESULT_MAX_ENUM);
	it->stageFlags |= binding.stageFlags;
}

void ShaderVk::FillDescriptorSetLayoutBindings(std::vector<vk::DescriptorSetLayoutBinding> &bindings)
{
  for (auto &bufferInfo : _uniformBuffers) {
    vk::DescriptorSetLayoutBinding binding;
    binding
      .setBinding(bufferInfo._binding)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(static_cast<uint32_t>(bufferInfo._layout->GetMultidimensionalArrayCount()))
      .setStageFlags(GetShaderStageFlags(_kind));

    AddSetLayoutBinding(bindings, binding);
  }

	for (auto &samplerInfo : _samplers) {
		vk::DescriptorSetLayoutBinding binding;
		binding
			.setBinding(samplerInfo._binding)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(static_cast<uint32_t>(samplerInfo._layout->GetMultidimensionalArrayCount()))
			.setStageFlags(GetShaderStageFlags(_kind));

		AddSetLayoutBinding(bindings, binding);
	}
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

std::vector<ShaderVk::UniformInfo> ShaderVk::GetUniformDescriptions(spirv_cross::Compiler const &reflected, 
	spirv_cross::SmallVector<spirv_cross::Resource> const &resources)
{
	std::vector<UniformInfo> uniforms;
  for (auto u : resources) {
    auto &type = reflected.get_type(u.type_id);

    uint32_t binding = reflected.get_decoration(u.id, spv::Decoration::DecorationBinding);
    std::string const &name = reflected.get_name(u.id);
		size_t size = type.basetype == spirv_cross::SPIRType::Struct
			? reflected.get_declared_struct_size(type) : 0;

    UniformInfo uniformInfo;
    uniformInfo._name = name;
    uniformInfo._binding = binding;
		uniformInfo._layout = GetLayoutFromSpirv(reflected, type, size);
    uniforms.push_back(std::move(uniformInfo));
  }
	return uniforms;
}

NAMESPACE_END(gr1)

