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

bool Shader::HasCommonVertexAttributes(util::LayoutElement const *otherLayout, std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices) const
{
	ASSERT(otherLayout->IsStruct());
	auto getLayoutIndexName = [&](util::StrId fieldId) {
		size_t matching = otherLayout->GetStructFieldIndex(fieldId);
		std::string name;
		if (matching != ~0ull)
			otherLayout->GetStructFieldName(matching);
		return std::make_pair(matching, name);
	};
	return HasCommonVertexAttributes(getLayoutIndexName, matchingIndices);
}

bool Shader::HasCommonVertexAttributes(
	std::function<std::pair<size_t, std::string>(util::StrId fieldId)> getMatchingIndexName, 
	std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices) const
{
	if (matchingIndices)
		matchingIndices->clear();
	for (uint32_t v = 0; v < _vertexLayout->GetStructFieldCount(); ++v) {
		util::StrId fieldId = _vertexLayout->GetStructFieldId(v);
		std::pair<size_t, std::string> indexName = getMatchingIndexName(fieldId);
		size_t matching = indexName.first;
		if (matching == ~0ull)
			continue;
		ASSERT(_vertexLayout->GetStructFieldName(v) == indexName.second || !indexName.second.size());
		if (!matchingIndices)
			return true;
		matchingIndices->emplace_back(static_cast<uint32_t>(v), static_cast<uint32_t>(matching));
	}
	return matchingIndices && matchingIndices->size();
}

NAMESPACE_END(gr1)

