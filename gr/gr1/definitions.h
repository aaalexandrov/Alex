#pragma once

#include "util/namespace.h"
#include "util/enumutl.h"
#include "util/rect.h"

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
};

enum class ColorFormat {
	Invalid,
	R8G8B8A8,
	B8G8R8A8,

	D24S8,
};

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

struct ImageData {
	glm::uvec4 _size = {};
	glm::uvec4 _pitch = {};
	uint8_t *_data = nullptr;

	ImageData() = default;
	ImageData(glm::uvec4 size, glm::uvec4 pitch, void *data) : _size(size), _pitch(pitch), _data(static_cast<uint8_t*>(data)) { ASSERT(_size.x && _pitch.x); }

	size_t GetOffset(glm::uvec4 pixelPos) const;

	glm::uvec4 GetEffectiveSize() const { return GetEffectiveSize(_size); }
	static glm::uvec4 GetEffectiveSize(glm::uvec4 s) { return util::VecMax(s, glm::uvec4(1, 1, 1, 1)); }


	static glm::uvec4 GetPackedPitch(glm::uvec4 imgSize, uint32_t elemSize);
	static void Copy(ImageData const &src, glm::uvec4 srcPos, ImageData const &dst, glm::uvec4 dstPos, glm::uvec4 size);
};


NAMESPACE_END(gr1)