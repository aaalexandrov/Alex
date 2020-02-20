#include "shader.h"

NAMESPACE_BEGIN(gr1)

void Shader::Init(std::string name, ShaderKind shaderKind, std::vector<uint8_t> const &contents)
{
	_name = name;
	_kind = shaderKind;
	LoadShader(contents);
}

bool Shader::HasCommonVertexAttributes(util::LayoutElement const *vertexLayout) const
{
	ASSERT(vertexLayout->IsStruct());
	for (int i = 0; i < vertexLayout->GetStructFieldCount(); ++i) {
		if (_vertexLayout->GetStructFieldIndex(vertexLayout->GetStructFieldName(i)) != ~0ull)
			return true;
	}
	return false;
}

NAMESPACE_END(gr1)

