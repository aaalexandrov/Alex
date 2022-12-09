#include "image.h"

#include "graphics_exception.h"

NAMESPACE_BEGIN(gr1)

void Image::Init(Usage usage, ColorFormat format, glm::uvec4 size, uint32_t mipLevels)
{
	ASSERT(mipLevels > 0);
	_usage = usage;
	_format = format;
	_size = size;
	_mipLevels = mipLevels;
}

ColorFormat Image::GetFirstAvailableDepthStencilFormat(std::vector<ColorFormat> const &desiredFormats)
{
	return GetFirstAvailableOption(desiredFormats, GetSupportedDepthStencilFormats());
}

uint32_t Image::GetColorFormatSize(ColorFormat format)
{
  switch (format) {
    case ColorFormat::D24S8:
    case ColorFormat::R8G8B8A8:
	case ColorFormat::B8G8R8A8:
	case ColorFormat::R8G8B8A8_srgb:
	case ColorFormat::B8G8R8A8_srgb:
		return 4;

	case ColorFormat::R8:
			return 1;
  }
  throw GraphicsException("Image::GetColorFormatSize(): Unknown color format!", -1);
}

std::unordered_map<ColorFormat, rttr::type> g_colorFormat2Type{{
	{ ColorFormat::R8G8B8A8     , rttr::type::get<glm::u8vec4>() },
	{ ColorFormat::R8G8B8A8_srgb, rttr::type::get<glm::u8vec4>() },
	{ ColorFormat::B8G8R8A8     , rttr::type::get<glm::u8vec4>() },
	{ ColorFormat::B8G8R8A8_srgb, rttr::type::get<glm::u8vec4>() },
	{ ColorFormat::R8           , rttr::type::get<uint8_t>()     },
}};

rttr::type Image::GetColorFormatType(ColorFormat format)
{
	return g_colorFormat2Type.at(format);
}

NAMESPACE_END(gr1)

