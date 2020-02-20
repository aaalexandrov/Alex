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
    uint32_t _binding;
		std::shared_ptr<util::LayoutElement> _layout;
  };

	Shader(Device &device) : Resource(device) {}

  virtual void Init(std::string name, ShaderKind shaderKind, std::vector<uint8_t> const &contents);
	virtual void LoadShader(std::vector<uint8_t> const &contents) = 0;

	inline std::string const &GetName() const { return _name; }
	inline ShaderKind GetShaderKind() const { return _kind; }
	inline std::shared_ptr<util::LayoutElement> const &GetVertexLayout() const { return _vertexLayout; }
	inline std::vector<UniformInfo> const &GetUniformBuffers() const { return _uniformBuffers; }
	inline std::vector<UniformInfo> const &GetSamplers() const { return _samplers; }

	bool HasCommonVertexAttributes(util::LayoutElement const *vertexLayout) const;

protected:
	std::string _name;
	ShaderKind _kind;
	std::shared_ptr<util::LayoutElement> _vertexLayout;
  std::vector<UniformInfo> _uniformBuffers;
	std::vector<UniformInfo> _samplers;
};

NAMESPACE_END(gr1)

