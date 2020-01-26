#include "shader.h"

NAMESPACE_BEGIN(gr1)

void Shader::Init(std::string name, ShaderKind shaderKind, std::vector<uint8_t> const &contents)
{
	_name = name;
	_kind = shaderKind;
	LoadShader(contents);
}

NAMESPACE_END(gr1)

