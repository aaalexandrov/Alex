#include "framework.h"

NAMESPACE_BEGIN(gf)

Framework::Framework()
{
	Init();
}

void Framework::Init()
{
	CreateDevice();
	_shaderProvider = std::make_shared<gr1::ShaderSourceProvider>();
	_defaultShaderOptions._shaderSource = _shaderProvider;
}

void Framework::SetDataRootPath(std::string path)
{
	_dataRootPath = path;
	std::replace(_dataRootPath.begin(), _dataRootPath.end(), '\\', '/');
	if (_dataRootPath.size() && _dataRootPath.back() != '/')
		_dataRootPath += '/';
}

std::string Framework::GetDataPath(std::string path) const
{
	return _dataRootPath + path;
}

std::shared_ptr<gr1::Shader> Framework::LoadShader(std::string path, gr1::ShaderKind::Enum kind, gr1::ShaderOptions const *shaderOptions)
{
	if (!shaderOptions)
		shaderOptions = &_defaultShaderOptions;

	path = GetDataPath(path);

	auto shader = _device->CreateResource<gr1::Shader>();
	shader->Init(path, kind, shaderOptions);

	return shader;
}

void Framework::CreateDevice()
{
	_device = _host.CreateDevice(0);
}



NAMESPACE_END(gf)

