#include "shader.h"

NAMESPACE_BEGIN(gr1)

void Shader::Init(std::string name, ShaderKind::Enum shaderKind, std::vector<uint8_t> const &contents, std::string entryPoint)
{
	_name = name;
	_entryPoint = entryPoint;
	_kind = shaderKind;
	LoadShader(contents);
}

auto Shader::GetParamInfo(Parameter::Kind paramKind, util::StrId paramId) const -> Parameter const *
{
	for (auto &param : _parameters[paramKind]) {
		if (param._id == paramId) {
			ASSERT(paramId.GetString() == param._name || !paramId.GetString().size());
			return &param;
		}
	}
	return nullptr;
}

auto Shader::GetParamInfo(Parameter::Kind paramKind, uint32_t binding) const -> Parameter const *
{
	for (auto &param : _parameters[paramKind]) {
		if (param._binding == binding)
			return &param;
	}
	return nullptr;
}

bool Shader::HasCommonVertexAttributes(util::LayoutElement const *layout, util::LayoutElement const *otherLayout, std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices)
{
	ASSERT(otherLayout->IsStruct());
	if (matchingIndices)
		matchingIndices->clear();
	for (uint32_t v = 0; v < layout->GetStructFieldCount(); ++v) {
		util::StrId fieldId = layout->GetStructFieldId(v);
		size_t matching = otherLayout->GetStructFieldIndex(fieldId);
		if (matching == ~0ull)
			continue;
		ASSERT(layout->GetStructFieldName(v) == otherLayout->GetStructFieldName(matching));
		if (!matchingIndices)
			return true;
		matchingIndices->emplace_back(static_cast<uint32_t>(v), static_cast<uint32_t>(matching));
	}
	return matchingIndices && matchingIndices->size();
}

NAMESPACE_END(gr1)

