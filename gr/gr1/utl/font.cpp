#include "font.h"
#include "../graphics_exception.h"
#include "../image.h"
#include "../buffer.h"
#include "util/str.h"


NAMESPACE_BEGIN(gr1)

Font::CharRange::CharRange(int32_t first, int32_t count)
{
	_firstCodePoint = first;
	_codePointCount = count;
	_stbChars.resize(count);
	_glyphIndices.resize(count);
	_stbRange.chardata_for_range = _stbChars.data();
}

bool Font::CharRange::ContainsCodePoint(int32_t codePoint) const
{
	return _firstCodePoint <= codePoint && codePoint < _firstCodePoint + _codePointCount;
}

int Font::GetFontIndices(std::shared_ptr<std::vector<uint8_t>> const &fontData)
{
	return stbtt_GetNumberOfFonts(fontData->data());
}

void Font::Init(std::shared_ptr<std::vector<uint8_t>> const &fontData, int fontIndex, float pixelHeight, std::vector<std::pair<int32_t, int32_t>> const &codePointRanges)
{
	_fontData = fontData;
	_fontIndex = fontIndex;

	if (!stbtt_InitFont(&_stbFont, _fontData->data(), _fontIndex))
		throw GraphicsException("Failed to load font", ~0ul);

	_scale = stbtt_ScaleForPixelHeight(&_stbFont, pixelHeight);
	
	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&_stbFont, &ascent, &descent, &lineGap);
	_ascent = ascent * _scale;
	_descent = -descent * _scale;
	_lineSpacing = _ascent + _descent + lineGap * _scale;

	ASSERT(util::IsEqual(_ascent + _descent, pixelHeight));

	int pixelCount = 0;
	for (auto &range : codePointRanges) {
		ASSERT(range.first < range.second);
		auto &charRange = _charRanges.emplace(std::pair(range.first, CharRange(range.first, range.second - range.first))).first->second;
		
		for (int cp = range.first; cp < range.second; ++cp) {
			util::RectI charRect;
			stbtt_GetCodepointBitmapBox(&_stbFont, cp, _scale, _scale, &charRect._min.x, &charRect._min.y, &charRect._max.x, &charRect._max.y);
			pixelCount += (charRect.GetSize().x + 1) * (charRect.GetSize().y + 1);

			charRange._glyphIndices[cp - range.first] = stbtt_FindGlyphIndex(&_stbFont, cp);
		}
	}
	
	int size = static_cast<int>(ceil(sqrt(pixelCount)));
	int pixHeight = static_cast<int>(ceil(pixelHeight));
	size = (size + pixHeight - 1) / pixHeight * pixHeight;
	std::vector<uint8_t> pixels(size * size);
	stbtt_pack_context packCtx;
	stbtt_PackBegin(&packCtx, pixels.data(), size, size, 0, 1, nullptr);

	for (auto &charRange : _charRanges) {
		auto &range = charRange.second;
		int result = stbtt_PackFontRange(&packCtx, _fontData->data(), _fontIndex, pixelHeight, range._firstCodePoint, range._codePointCount, range._stbChars.data());
		if (!result)
			throw GraphicsException("Failed to render font to bitmap", ~0ul);
	}

	stbtt_PackEnd(&packCtx);

	auto stagingBuf = _device->CreateResource<Buffer>();
	stagingBuf->Init(Buffer::Usage::Staging, pixels);

	_texture = _device->CreateResource<Image>();
	_texture->Init(Image::Usage::Texture, ColorFormat::R8, glm::uvec4(size, size, 0, 0), 1);

	auto copyTexPass = _device->CreateResource<ImageBufferCopyPass>();
	copyTexPass->Init(stagingBuf, _texture);

	_device->GetExecutionQueue().EnqueuePass(copyTexPass);
}

static util::StrId s_indicesId("indices", util::StrId::AddToRepository), s_verticesId("vertices", util::StrId::AddToRepository);

void Font::SetRenderingData(uint32_t sizeChars, std::string positionAttr, std::string texCoordAttr, std::string colorAttr)
{
	auto indicesLayout = util::CreateLayoutArray(rttr::type::get<uint32_t>(), sizeChars * 6);
	auto verticesLayout = util::CreateLayoutArray(
		util::CreateLayoutStruct(positionAttr, rttr::type::get<glm::vec2>(), 
			texCoordAttr, rttr::type::get<glm::vec2>(), 
			colorAttr, rttr::type::get<glm::u8vec4>()), sizeChars * 4);
	auto bufLayout = util::CreateLayoutStruct(s_indicesId.GetString(), indicesLayout, s_verticesId.GetString(), verticesLayout);

	_stagingBuffer = _device->CreateResource<Buffer>();
	_stagingBuffer->Init(Buffer::Usage::Staging, bufLayout);

	_renderBuffer = _device->CreateResource<Buffer>();
	_renderBuffer->Init(Buffer::Usage::Vertex | Buffer::Usage::Index, bufLayout);

	_bufferData.resize(bufLayout->GetSize());

	_copyStagingToRenderBuffer = _device->CreateResource<BufferCopyPass>();
	_copyStagingToRenderBuffer->Init(_stagingBuffer, _renderBuffer);

	_maxChars = sizeChars;
	_usedChars = 0;

	auto vertElemLayout = verticesLayout->GetArrayElement();
	_offsetPosition = static_cast<uint32_t>(vertElemLayout->GetMemberElementAndOffset(util::StrId(positionAttr)).second);
	_offsetTexCoord = static_cast<uint32_t>(vertElemLayout->GetMemberElementAndOffset(util::StrId(texCoordAttr)).second);
	_offsetColor = static_cast<uint32_t>(vertElemLayout->GetMemberElementAndOffset(util::StrId(colorAttr)).second);
	_vertexStride = static_cast<uint32_t>(vertElemLayout->GetSize());
}

void Font::Clear()
{
	_usedChars = 0;
}

auto Font::AddText(glm::vec2 &pixelPos, std::string text, int32_t prevCodePoint, glm::vec4 color) -> TextId
{
	uint8_t const *str = reinterpret_cast<uint8_t const*>(text.c_str());
	uint8_t const *strEnd = reinterpret_cast<uint8_t const *>(text.c_str() + text.length());

	uint32_t *indices = _stagingBuffer->GetBufferLayout()->GetMemberPtr<uint32_t>(_bufferData.data(), s_indicesId, 0);
	uint8_t *vertices = reinterpret_cast<uint8_t*>(_stagingBuffer->GetBufferLayout()->GetMemberPtr<void>(_bufferData.data(), s_verticesId));
	TextId id{ _usedChars };
	glm::u8vec4 packedColor = glm::round(glm::clamp(color, 0.0f, 1.0f) * 255.0f);
	while (str < strEnd) {
		int32_t cp = util::ReadUnicodePoint(str, strEnd);
		if (!cp) {
			ASSERT(str == strEnd);
			break;
		}
		bool result = SetCharQuad(indices, vertices, _usedChars, cp, prevCodePoint, pixelPos, packedColor);
		ASSERT(result);
		if (result) {
			++_usedChars;
		}
		prevCodePoint = cp;
	}

	return id;
}

util::RectF Font::MeasureText(glm::vec2 &pixelPos, std::string text, int32_t prevCodePoint)
{
	uint8_t const *str = reinterpret_cast<uint8_t const*>(text.c_str());
	uint8_t const *strEnd = reinterpret_cast<uint8_t const *>(text.c_str() + text.length());

	util::RectF rect{ pixelPos, pixelPos };
	glm::vec2 orgPos = pixelPos;

	while (str < strEnd) {
		int32_t cp = util::ReadUnicodePoint(str, strEnd);
		if (!cp) {
			ASSERT(str == strEnd);
			break;
		}

		stbtt_aligned_quad quad;
		if (!FillAlignedQuad(cp, prevCodePoint, pixelPos, quad)) {
			rect = util::RectF();
			pixelPos = orgPos;
			break;
		}

		rect._min = glm::min(rect._min, glm::vec2(quad.x0, quad.y0));
		rect._max = glm::max(rect._max, glm::vec2(quad.x1, quad.y1));

		prevCodePoint = cp;
	}

	return rect;
}

void Font::SetDataToDrawCommand(RenderDrawCommand *cmdDraw)
{
	size_t indOffset = _stagingBuffer->GetBufferLayout()->GetMemberElementAndOffset(s_indicesId).second;
	size_t vertOffset = _stagingBuffer->GetBufferLayout()->GetMemberElementAndOffset(s_verticesId).second;
	uint8_t *mapped = reinterpret_cast<uint8_t*>(_stagingBuffer->Map());

	memcpy(mapped + indOffset, _bufferData.data() + indOffset, _usedChars * 6 * sizeof(uint32_t));
	memcpy(mapped + vertOffset, _bufferData.data() + vertOffset, _usedChars * 4 * _vertexStride);

	_stagingBuffer->Unmap();

	_device->GetExecutionQueue().EnqueuePass(_copyStagingToRenderBuffer);

	bool addIndex = true, addVertex = true;
	for (int i = cmdDraw->GetBufferCount() - 1; i >= 0; --i) {
		auto &bufData = cmdDraw->GetBufferData(i);
		if (bufData._buffer == _renderBuffer) {
			if (bufData.IsIndex())
				addIndex = false;
			if (bufData.IsVertex())
				addVertex = false;
		} else {
			if (bufData.IsIndex() || bufData.IsVertex())
				cmdDraw->RemoveBuffer(i);
		}
	}

	if (addIndex)
		cmdDraw->AddBuffer(_renderBuffer, util::StrId(), indOffset, false, _renderBuffer->GetBufferLayout()->GetElement(s_indicesId)->AsShared());
	if (addVertex)
		cmdDraw->AddBuffer(_renderBuffer, util::StrId(), vertOffset, false, _renderBuffer->GetBufferLayout()->GetElement(s_verticesId)->AsShared());

	auto samplerData = cmdDraw->GetSamplerData(0);
	if (samplerData._image != _texture) {
		util::StrId samplerId = cmdDraw->GetShader(ShaderKind::Fragment)->GetSamplerInfo(samplerData._bindings[static_cast<int>(ShaderKind::Fragment)])->_id;
		cmdDraw->RemoveSampler(0);
		cmdDraw->AddSampler(samplerData._sampler, _texture, samplerId);
	}

	cmdDraw->SetDrawCounts(_usedChars * 6);
	cmdDraw->SetPrimitiveKind(PrimitiveKind::TriangleList);
}

auto Font::GetCharRange(int32_t codePoint) -> CharRange const *
{
	auto it = _charRanges.upper_bound(codePoint);
	if (it != _charRanges.begin())
		--it;
	if (it == _charRanges.end() || !it->second.ContainsCodePoint(codePoint))
		return nullptr;
	return &it->second;
}

bool Font::FillAlignedQuad(int32_t codePoint, int32_t prevCodePoint, glm::vec2 &pixelPos, stbtt_aligned_quad &quad)
{
	CharRange const *range, *prevRange;
	range = GetCharRange(codePoint);
	if (!range)
		return false;
	prevRange = GetCharRange(prevCodePoint);
	float kern = 0;
	if (prevRange) {
		kern = stbtt_GetGlyphKernAdvance(&_stbFont, prevRange->GetGlyphIndex(prevCodePoint), range->GetGlyphIndex(codePoint)) * _scale;
	}
	pixelPos.x += kern;

	glm::ivec2 size = _texture->GetSize();

	stbtt_GetPackedQuad(range->_stbChars.data(), size.x, size.y, codePoint - range->_firstCodePoint, &pixelPos.x, &pixelPos.y, &quad, true);

	return true;
}

bool Font::SetCharQuad(uint32_t *indices, uint8_t *vertices, uint32_t quadIndex, int32_t codePoint, int32_t prevCodePoint, glm::vec2 &pixelPos, glm::u8vec4 color)
{
	stbtt_aligned_quad quad;
	if (!FillAlignedQuad(codePoint, prevCodePoint, pixelPos, quad))
		return false;

	vertices += quadIndex * 4 *_vertexStride;
	indices += quadIndex * 6;

	*reinterpret_cast<glm::vec2*>(vertices + _offsetPosition) = glm::vec2(quad.x0, quad.y0);
	*reinterpret_cast<glm::vec2*>(vertices + _offsetTexCoord) = glm::vec2(quad.s0, quad.t0);
	*reinterpret_cast<glm::u8vec4*>(vertices + _offsetColor) = color;
	vertices += _vertexStride;

	*reinterpret_cast<glm::vec2*>(vertices + _offsetPosition) = glm::vec2(quad.x1, quad.y0);
	*reinterpret_cast<glm::vec2*>(vertices + _offsetTexCoord) = glm::vec2(quad.s1, quad.t0);
	*reinterpret_cast<glm::u8vec4*>(vertices + _offsetColor) = color;
	vertices += _vertexStride;

	*reinterpret_cast<glm::vec2*>(vertices + _offsetPosition) = glm::vec2(quad.x1, quad.y1);
	*reinterpret_cast<glm::vec2*>(vertices + _offsetTexCoord) = glm::vec2(quad.s1, quad.t1);
	*reinterpret_cast<glm::u8vec4*>(vertices + _offsetColor) = color;
	vertices += _vertexStride;

	*reinterpret_cast<glm::vec2*>(vertices + _offsetPosition) = glm::vec2(quad.x0, quad.y1);
	*reinterpret_cast<glm::vec2*>(vertices + _offsetTexCoord) = glm::vec2(quad.s0, quad.t1);
	*reinterpret_cast<glm::u8vec4*>(vertices + _offsetColor) = color;

	indices[0] = quadIndex * 4;
	indices[1] = quadIndex * 4 + 2;
	indices[2] = quadIndex * 4 + 1;

	indices[3] = quadIndex * 4;
	indices[4] = quadIndex * 4 + 3;
	indices[5] = quadIndex * 4 + 2;

	return true;
}

NAMESPACE_END(gr1)

