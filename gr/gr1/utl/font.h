#pragma once

#include "../execution_queue.h"
#include "../render_commands.h"
#include "stb/stb_truetype.h"

NAMESPACE_BEGIN(gr1)

class Font {
public:
	struct TextId {
		uint32_t _bufferPos;
	};

	Font(Device &device) : _device(&device) {}

	static int GetFontIndices(std::shared_ptr<std::vector<uint8_t>> const &fontData);
	void Init(std::shared_ptr<std::vector<uint8_t>> const &fontData, int fontIndex, float pixelHeight, std::vector<std::pair<int32_t, int32_t>> const &codePointRanges);

	void SetRenderingData(uint32_t sizeChars, std::string positionAttr, std::string texCoordAttr, std::string colorAttr);

	void Clear();
	TextId AddText(glm::vec2 &pixelPos, std::string text, int32_t prevCodePoint = 0, glm::vec4 color = glm::vec4(1));

	util::RectF MeasureText(glm::vec2 &pixelPos, std::string text, int32_t prevCodePoint = 0);

	void SetDataToDrawCommand(RenderDrawCommand *cmdDraw);

	std::shared_ptr<std::vector<uint8_t>> const &GetFontData() const { return _fontData; }
	float GetAscent() const { return _ascent; }
	float GetDescent() const { return _descent; }
	float GetLineSpacing() const { return _lineSpacing; }

	std::shared_ptr<Image> const &GetTexture() const { return _texture; }

protected:
	struct CharRange {
		int32_t _firstCodePoint, _codePointCount;
		stbtt_pack_range _stbRange = {};
		std::vector<stbtt_packedchar> _stbChars;
		std::vector<int32_t> _glyphIndices;

		CharRange(int32_t first, int32_t count);
		bool ContainsCodePoint(int32_t codePoint) const;
		int32_t GetGlyphIndex(int32_t codePoint) const { return _glyphIndices[codePoint - _firstCodePoint]; }
	};

	CharRange const *GetCharRange(int32_t codePoint);
	bool FillAlignedQuad(int32_t codePoint, int32_t prevCodePoint, glm::vec2 &pixelPos, stbtt_aligned_quad &quad);
	bool SetCharQuad(uint32_t *indices, uint8_t *vertices, uint32_t quadIndex, int32_t codePoint, int32_t prevCodePoint, glm::vec2 &pixelPos, glm::u8vec4 color);

	Device *_device;
	std::shared_ptr<Image> _texture;
	uint32_t _usedChars, _maxChars;
	std::shared_ptr<Buffer> _stagingBuffer, _renderBuffer;
	std::shared_ptr<BufferCopyPass> _copyStagingToRenderBuffer;
	std::vector<uint8_t> _bufferData;

	uint32_t _offsetPosition, _offsetTexCoord, _offsetColor, _vertexStride;
	
	std::shared_ptr<std::vector<uint8_t>> _fontData;
	int _fontIndex = 0;
	float _ascent, _descent, _lineSpacing;
	float _scale;
	stbtt_fontinfo _stbFont = {};
	std::map<int32_t, CharRange> _charRanges;
};

NAMESPACE_END(gr1)