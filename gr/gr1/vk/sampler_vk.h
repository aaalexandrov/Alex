#include "../sampler.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class SamplerVk : public Sampler {
	RTTR_ENABLE(Sampler)
public:
	SamplerVk(Device &device) : Sampler(device) {}

	void Init(SamplerData const *samplerData = nullptr) override;
public:
	vk::UniqueSampler CreateVkSampler(SamplerData &samplerData);

	vk::UniqueSampler _sampler;

	static util::ValueRemapper<Filter, vk::Filter> s_Filter2Vk;
	static util::ValueRemapper<MipMapMode, vk::SamplerMipmapMode> s_MipMapMode2Vk;
	static util::ValueRemapper<AddressMode, vk::SamplerAddressMode> s_AddressMode2Vk;
	static util::ValueRemapper<BorderColor, vk::BorderColor> s_BorderColor2Vk;
};

NAMESPACE_END(gr1)