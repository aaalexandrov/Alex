#pragma once

#include "resource.h"
#include "util/enumutl.h"
#include "util/layout.h"
#include <string>
#include <memory>

NAMESPACE_BEGIN(gr1)

enum class ShaderKind {
  Vertex,
  Fragment,
	Invalid,

  Count = Invalid
};

enum class ShaderKindBits {
	None = 0,
	Vertex = 1 << static_cast<size_t>(ShaderKind::Vertex),
	Fragment = 1 << static_cast<size_t>(ShaderKind::Fragment),
};

DEFINE_ENUM_BIT_OPERATORS(gr1::ShaderKindBits)

enum class TextureDimension {
	None,
	Dim1D,
	Dim2D,
	Dim3D,
	Cube,
};

template <TextureDimension Dim>
struct TextureKind {};

class Shader;
template <typename Data>
using ShaderKindsArray = std::array<Data, static_cast<int>(ShaderKind::Count)>;

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

protected:
	std::string _name;
	ShaderKind _kind;
	std::shared_ptr<util::LayoutElement> _vertexLayout;
  std::vector<UniformInfo> _uniformBuffers;
	std::vector<UniformInfo> _samplers;
};

NAMESPACE_END(gr1)

