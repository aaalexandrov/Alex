#pragma once

#include "util/namespace.h"
#include "util/enumutl.h"
#include "buffer_desc.h"
#include "resource.h"
#include <string>
#include <memory>

NAMESPACE_BEGIN(gr1)

enum class ShaderKind {
  None = 0,
  Vertex = 1,
  Fragment = 2,

  Count = Fragment
};

DEFINE_ENUM_BIT_OPERATORS(ShaderKind)

class Shader : public Resource {
	RTTR_ENABLE(Resource)
public:
  struct UniformBufferInfo {
    std::string _name;
    uint32_t _binding;
    BufferDescPtr _uniformDesc = BufferDesc::Create();
  };

	Shader(Device &device) : Resource(device) {}

  virtual void Init(std::string name, ShaderKind shaderKind, std::vector<uint8_t> const &contents);
	rttr::type GetStateTransitionPassType() override { return rttr::type::get<void>(); }
	virtual void LoadShader(std::vector<uint8_t> const &contents) = 0;

	inline std::string const &GetName() const { return _name; }
	inline ShaderKind GetShaderKind() const { return _kind; }
	inline BufferDesc *GetVertexDescription() const { return _vertexDesc.get(); }
	inline std::vector<UniformBufferInfo> const &GetUniformBuffers() const { return _uniformBuffers; }

protected:
	std::string _name;
	ShaderKind _kind;
  BufferDescPtr _vertexDesc = BufferDesc::Create();
  std::vector<UniformBufferInfo> _uniformBuffers;
};

NAMESPACE_END(gr1)
