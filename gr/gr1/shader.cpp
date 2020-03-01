#include "shader.h"
#include "util/file.h"

NAMESPACE_BEGIN(gr1)

std::string ShaderSourceProvider::GetPath(std::string include, IncludeType incType, std::string requester)
{
	return _pathTranslate(include, incType, requester);
}

std::shared_ptr<std::vector<uint8_t>> const &ShaderSourceProvider::GetSource(std::string source)
{
	std::lock_guard<std::mutex> lock(_mutex);
	auto it = _loadedSources.find(source);
	if (it == _loadedSources.end()) {
		auto src = std::make_shared<std::vector<uint8_t>>();
		*src = _shaderSource(source);
		it = _loadedSources.insert(std::make_pair(source, src)).first;
	}
	return it->second;
}

void ShaderSourceProvider::UnloadSource(std::string source)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_loadedSources.erase(source);
}

void ShaderSourceProvider::UnloadAllSources()
{
	std::lock_guard<std::mutex> lock(_mutex);
	_loadedSources.clear();
}

std::string ShaderSourceProvider::DefaultPathTranslate(std::string include, IncludeType incType, std::string requester)
{
	return util::GetPathDir(requester) + "/" + include;
}

std::vector<uint8_t> ShaderSourceProvider::DefaultShaderSource(std::string path)
{
	return util::ReadFile(path);
}


static ShaderOptions s_defaultShaderOptions;

void Shader::Init(std::string name, ShaderKind::Enum shaderKind, ShaderOptions const *shaderOptions, std::string entryPoint)
{
	_name = name;
	_entryPoint = entryPoint;
	_kind = shaderKind;
	if (!shaderOptions)
		shaderOptions = &s_defaultShaderOptions;
	LoadShader(name, *shaderOptions);
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

