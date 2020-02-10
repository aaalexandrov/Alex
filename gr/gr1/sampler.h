#include "resource.h"
#include "render_state.h"

NAMESPACE_BEGIN(gr1)

class Sampler : public Resource {
	RTTR_ENABLE(Resource)
public:
	enum class Filter {
		Nearest,
		Linear,
		Cubic,
	};

	enum class MipMapMode {
		Nearest,
		Linear,
	};

	enum class AddressMode {
		Repeat,
		ClampToEdge,
		ClampToBorder,
		MirroredRepeat,
		MirrorClampToEdge,
	};

	enum class BorderColor {
		TransparentBlackFloat,
		TransparentBlackInt,
		OpaqueBlackFloat,
		OpaqueBlackInt,
		OpaqueWhiteFloat,
		OpaqueWhiteInt,
	};

	struct SamplerData {
		Filter _magFilter = Filter::Linear, _minFilter = Filter::Linear;
		MipMapMode _mipMapMode = MipMapMode::Linear;
		std::array<AddressMode, 3> _addressModes{ AddressMode::Repeat, AddressMode::Repeat, AddressMode::Repeat };
		float _mipLodBias = 0;
		bool _anisotropic = false;
		float _maxAnisotropy = 16;
		bool _compareEnable = false;
		CompareFunc _compareFunc = CompareFunc::Always;
		float _minLod = 0, _maxLod = 1;
		BorderColor _borderColor = BorderColor::TransparentBlackFloat;
		bool _normalizedCoordinates = true;

		size_t GetHash() const;
		bool operator ==(SamplerData const &other) const;
	};

	Sampler(Device &device) : Resource(device) {}

	virtual void Init();

	SamplerData const &GetSamplerData() const { return _data; }

public:
	SamplerData _data;
};

NAMESPACE_END(gr1)

NAMESPACE_BEGIN(std)

template <> struct hash<gr1::Sampler::SamplerData> {
	size_t operator()(gr1::Sampler::SamplerData const &samplerData) { return samplerData.GetHash(); }
};

NAMESPACE_END(std)