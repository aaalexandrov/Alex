#pragma once

#include "definitions.h"
#include "resource.h"
#include "util/layout.h"
#include <string>
#include <memory>

NAMESPACE_BEGIN(gr1)


class Shader : public Resource {
	RTTR_ENABLE(Resource)
public:
  struct UniformInfo {
    std::string _name;
		util::StrId _id;
    uint32_t _binding;
		std::shared_ptr<util::LayoutElement> _layout;

		UniformInfo(std::string name, uint32_t binding, std::shared_ptr<util::LayoutElement>const &layout)
			: _name(name), _id(name), _binding(binding), _layout(layout) {}
  };

	Shader(Device &device) : Resource(device) {}

  virtual void Init(std::string name, ShaderKind shaderKind, std::vector<uint8_t> const &contents, std::string entryPoint = "main");
	virtual void LoadShader(std::vector<uint8_t> const &contents) = 0;

	inline std::string const &GetName() const { return _name; }
	inline ShaderKind GetShaderKind() const { return _kind; }
	inline std::shared_ptr<util::LayoutElement> const &GetVertexLayout() const { return _vertexLayout; }

	inline std::vector<UniformInfo> const &GetUniformBuffers() const { return _uniformBuffers; }
	inline std::vector<UniformInfo> const &GetSamplers() const { return _samplers; }

	UniformInfo const *GetUniformInfo(util::StrId bufferId) const { return GetUniformInfo([&](auto const &b) { return b._id == bufferId; }, _uniformBuffers); }
	UniformInfo const *GetSamplerInfo(util::StrId samplerId) const { return GetUniformInfo([&](auto const &b) { return b._id == samplerId; }, _samplers); }

	UniformInfo const *GetUniformInfo(uint32_t binding) const { return GetUniformInfo([&](auto const &b) { return b._binding == binding; }, _uniformBuffers); }
	UniformInfo const *GetSamplerInfo(uint32_t binding) const { return GetUniformInfo([&](auto const &b) { return b._binding == binding; }, _samplers); }

	bool HasCommonVertexAttributes(util::LayoutElement const *otherLayout, std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices) const;

protected:
	static UniformInfo const *GetUniformInfo(std::function<bool(UniformInfo const &)> filter, std::vector<UniformInfo> const &infos);

	std::string _name, _entryPoint;
	ShaderKind _kind;
	std::shared_ptr<util::LayoutElement> _vertexLayout;
  std::vector<UniformInfo> _uniformBuffers;
	std::vector<UniformInfo> _samplers;
};

NAMESPACE_END(gr1)

