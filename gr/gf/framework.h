#pragma once

#include "gr1/host.h"
#include "gr1/execution_queue.h"
#include "gr1/device.h"
#include "gr1/shader.h"

NAMESPACE_BEGIN(gf)

class Framework {
public:
	Framework();

	void SetDataRootPath(std::string path);
	std::string const &GetDataRootPath() const { return _dataRootPath; }

	std::shared_ptr<gr1::ShaderSourceProvider> const &GetShaderSourceProvider() const { return _shaderProvider; }
	std::shared_ptr<gr1::Shader> LoadShader(std::string path, gr1::ShaderKind::Enum kind, gr1::ShaderOptions const *shaderOptions = nullptr);


public:
	void Init();
	void CreateDevice();

	std::string GetDataPath(std::string path) const;

	gr1::Host _host;
	std::shared_ptr<gr1::Device> _device;

	std::string _dataRootPath;
	std::shared_ptr<gr1::ShaderSourceProvider> _shaderProvider;
	gr1::ShaderOptions _defaultShaderOptions{nullptr};
};

NAMESPACE_END(gf)