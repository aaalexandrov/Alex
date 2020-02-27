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
  struct Parameter {
		enum Kind {
			UniformBuffer,
			Sampler,
			Invalid,

			VertexBuffer,
			IndexBuffer,

			Count = Invalid,
			First = UniformBuffer,
		};

    std::string _name;
		util::StrId _id;
    uint32_t _binding;
		std::shared_ptr<util::LayoutElement> _layout;

		Parameter(std::string name, uint32_t binding, std::shared_ptr<util::LayoutElement>const &layout)
			: _name(name), _id(name), _binding(binding), _layout(layout) {}
  };

	Shader(Device &device) : Resource(device) {}

  virtual void Init(std::string name, ShaderKind::Enum shaderKind, std::vector<uint8_t> const &contents, std::string entryPoint = "main");
	virtual void LoadShader(std::vector<uint8_t> const &contents) = 0;

	inline std::string const &GetName() const { return _name; }
	inline ShaderKind::Enum GetShaderKind() const { return _kind; }
	inline std::shared_ptr<util::LayoutElement> const &GetVertexLayout() const { return _vertexLayout; }

	inline std::vector<Parameter> const &GetParameters(Parameter::Kind paramKind) const { return _parameters[paramKind]; }

	Parameter const *GetParamInfo(Parameter::Kind paramKind, util::StrId paramId) const;
	Parameter const *GetParamInfo(Parameter::Kind paramKind, uint32_t binding) const;

	bool HasCommonVertexAttributes(
		util::LayoutElement const *otherLayout, 
		std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices) const 
	{ return HasCommonVertexAttributes(_vertexLayout.get(), otherLayout, matchingIndices); }

	static bool HasCommonVertexAttributes(
		util::LayoutElement const *layout, 
		util::LayoutElement const *otherLayout, 
		std::vector<std::pair<uint32_t, uint32_t>> *matchingIndices);

protected:
	std::string _name, _entryPoint;
	ShaderKind::Enum _kind;
	std::shared_ptr<util::LayoutElement> _vertexLayout;
	std::array<std::vector<Parameter>, Parameter::Kind::Count> _parameters;
};

NAMESPACE_END(gr1)

