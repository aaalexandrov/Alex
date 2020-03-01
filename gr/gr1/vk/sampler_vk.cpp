#include "sampler_vk.h"
#include "device_vk.h"
#include "render_state_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<SamplerVk>("SamplerVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

void SamplerVk::Init(SamplerData const *samplerData)
{
	Sampler::Init(samplerData);
	_sampler = CreateVkSampler(_data);
}

util::ValueRemapper<SamplerVk::Filter, vk::Filter> SamplerVk::s_Filter2Vk{ {
		{ Filter::Nearest, vk::Filter::eNearest },
		{ Filter::Linear, vk::Filter::eLinear },
		{ Filter::Cubic, vk::Filter::eCubicIMG },
	} };

util::ValueRemapper<SamplerVk::MipMapMode, vk::SamplerMipmapMode> SamplerVk::s_MipMapMode2Vk{ {
		{ MipMapMode::Nearest, vk::SamplerMipmapMode::eNearest },
		{ MipMapMode::Linear, vk::SamplerMipmapMode::eLinear },
	} };

util::ValueRemapper<SamplerVk::AddressMode, vk::SamplerAddressMode> SamplerVk::s_AddressMode2Vk{ {
		{ AddressMode::Repeat, vk::SamplerAddressMode::eRepeat },
		{ AddressMode::ClampToEdge, vk::SamplerAddressMode::eClampToEdge },
		{ AddressMode::ClampToBorder, vk::SamplerAddressMode::eClampToBorder },
		{ AddressMode::MirroredRepeat, vk::SamplerAddressMode::eMirroredRepeat },
		{ AddressMode::MirrorClampToEdge, vk::SamplerAddressMode::eMirrorClampToEdge },
	} };

util::ValueRemapper<SamplerVk::BorderColor, vk::BorderColor> SamplerVk::s_BorderColor2Vk{ {
		{ BorderColor::TransparentBlackFloat, vk::BorderColor::eFloatTransparentBlack	},
		{ BorderColor::TransparentBlackInt, vk::BorderColor::eIntTransparentBlack	},
		{ BorderColor::OpaqueBlackFloat, vk::BorderColor::eFloatOpaqueBlack	},
		{ BorderColor::OpaqueBlackInt, vk::BorderColor::eIntOpaqueBlack	},
		{ BorderColor::OpaqueWhiteFloat, vk::BorderColor::eFloatOpaqueWhite	},
		{ BorderColor::OpaqueWhiteInt, vk::BorderColor::eIntOpaqueWhite	},
	} };

vk::UniqueSampler SamplerVk::CreateVkSampler(SamplerData &samplerData)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	vk::SamplerCreateInfo samplerInfo;
	samplerInfo
		.setMagFilter(s_Filter2Vk.ToDst(samplerData._magFilter))
		.setMinFilter(s_Filter2Vk.ToDst(samplerData._minFilter))
		.setMipmapMode(s_MipMapMode2Vk.ToDst(samplerData._mipMapMode))
		.setAddressModeU(s_AddressMode2Vk.ToDst(samplerData._addressModes[0]))
		.setAddressModeV(s_AddressMode2Vk.ToDst(samplerData._addressModes[1]))
		.setAddressModeW(s_AddressMode2Vk.ToDst(samplerData._addressModes[2]))
		.setMipLodBias(samplerData._mipLodBias)
		.setAnisotropyEnable(samplerData._maxAnisotropy > 1.0f)
		.setMaxAnisotropy(samplerData._maxAnisotropy)
		.setCompareEnable(samplerData._compareFunc <= CompareFunc::Last)
		.setCompareOp(s_compareFunc2Vk.ToDst(std::min(samplerData._compareFunc, CompareFunc::Last)))
		.setMinLod(samplerData._minLod)
		.setMaxLod(samplerData._maxLod)
		.setBorderColor(s_BorderColor2Vk.ToDst(samplerData._borderColor))
		.setUnnormalizedCoordinates(!samplerData._normalizedCoordinates);
	return deviceVk->_device->createSamplerUnique(samplerInfo, deviceVk->AllocationCallbacks());
}

NAMESPACE_END(gr1)