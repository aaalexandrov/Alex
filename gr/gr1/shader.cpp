#include "shader.h"

NAMESPACE_BEGIN(gr1)

void Shader::Init(std::string name, ShaderKind shaderKind, std::vector<uint8_t> const &contents, std::string entryPoint)
{
	_name = name;
	_entryPoint = entryPoint;
	_kind = shaderKind;
	LoadShader(contents);
}

bool Shader::HasCommonVertexAttributes(util::LayoutElement const *otherLayout, std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices) const
{
	ASSERT(otherLayout->IsStruct());
	if (matchingIndices)
		matchingIndices->clear();
	for (uint32_t v = 0; v < _vertexLayout->GetStructFieldCount(); ++v) {
		util::StrId fieldId = _vertexLayout->GetStructFieldId(v);
		size_t matching = otherLayout->GetStructFieldIndex(fieldId);
		if (matching == ~0ull)
			continue;
		ASSERT(_vertexLayout->GetStructFieldName(v) == otherLayout->GetStructFieldName(matching));
		if (!matchingIndices)
			return true;
		matchingIndices->emplace_back(static_cast<uint32_t>(v), static_cast<uint32_t>(matching));
	}
	return matchingIndices && matchingIndices->size();
}

auto Shader::GetUniformInfo(std::function<bool(UniformInfo const &)> filter, std::vector<UniformInfo> const &infos) -> UniformInfo const *
{
	for (auto &info : infos) {
		if (filter(info))
			return &info;
	}
	return nullptr;
}

NAMESPACE_END(gr1)

