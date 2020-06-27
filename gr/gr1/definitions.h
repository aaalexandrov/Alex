#pragma once

#include "util/namespace.h"
#include "util/enumutl.h"
#include "util/geom.h"

NAMESPACE_BEGIN(gr1)

enum class ContentTreatment {
	Keep,
	Clear,
	DontCare,
};

enum class ResourceState {
	Initial = 1 << 0,
	ShaderRead = 1 << 1,
	TransferRead = 1 << 2,
	TransferWrite = 1 << 3,
	RenderWrite = 1 << 4,
	PresentRead = 1 << 5,
	PresentAcquired = 1 << 6,
	Invalidated = 1 << 7,
};

enum class ColorFormat {
	Invalid,
	R8G8B8A8,
	B8G8R8A8,
	R8,

	D24S8,
};

enum class IncludeType {
	Relative, // "file"
	Standard, // <file>
};

namespace ShaderKind {
	enum Enum {
		Vertex,
		Fragment,
		Invalid,

		First = Vertex,
		Count = Invalid
	};
};

enum class ShaderKindBits {
	None = 0,
	Vertex = 1 << ShaderKind::Vertex,
	Fragment = 1 << ShaderKind::Fragment,
};

DEFINE_ENUM_BIT_OPERATORS(ShaderKindBits)

struct Texture1D {};
struct Texture2D {};
struct Texture3D {};
struct TextureCube {};

template <typename Data>
using ShaderKindsArray = std::array<Data, ShaderKind::Count>;

struct PipelineResource {
	enum Kind : uint32_t {
		Buffer,
		Sampler,
		Invalid,

		Count = Invalid,
	};
};

enum class DependencyType {
	None = 0,
	Input = 1,
	Output = 2,
};

class Resource;
using DependencyFunc = std::function<void(Resource*, ResourceState)>;

enum class CompareFunc {
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always,

	Last = Always,
	Invalid,
};

enum class StencilFunc {
	Keep,
	Zero,
	Replace,
	IncrementAndClamp,
	DecrementAndClamp,
	Invert,
	IncrementAndWrap,
	DecrementAndWrap,
};

enum class FrontFaceMode {
	CCW,
	CW,
};

enum class CullMask {
	None,
	Front = 1,
	Back = 2,
	FrontAndBack = 3,
};

enum class BlendFunc
{
	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max,
};

enum class BlendFactor
{
	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	DstColor,
	OneMinusDstColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha,
	OneMinusConstantAlpha,
	SrcAlphaSaturate,
	Src1Color,
	OneMinusSrc1Color,
	Src1Alpha,
	OneMinusSrc1Alpha,
};

enum class ColorComponentMask
{
	None,
	R = 1,
	G = 2,
	B = 4,
	A = 8,
	RGBA = 15,
};

DEFINE_ENUM_BIT_OPERATORS(ColorComponentMask)

struct ImageData {
	glm::uvec4 _size = {};
	glm::uvec4 _pitch = {};
	uint8_t *_data = nullptr;

	ImageData() = default;
	ImageData(glm::uvec4 size, glm::uvec4 pitch, void *data) : _size(size), _pitch(pitch), _data(static_cast<uint8_t*>(data)) { ASSERT(_size.x && _pitch.x); }

	size_t GetOffset(glm::uvec4 pixelPos) const;

	glm::uvec4 GetEffectiveSize() const { return GetEffectiveSize(_size); }
	static glm::uvec4 GetEffectiveSize(glm::uvec4 s) { return glm::max(s, glm::uvec4(1, 1, 1, 1)); }

	static glm::uvec4 GetPackedPitch(glm::uvec4 imgSize, uint32_t elemSize);
	static void Copy(ImageData const &src, glm::uvec4 srcPos, ImageData const &dst, glm::uvec4 dstPos, glm::uvec4 size);
};

enum class PrimitiveKind {
	TriangleList,
	TriangleStrip,
};


NAMESPACE_END(gr1)