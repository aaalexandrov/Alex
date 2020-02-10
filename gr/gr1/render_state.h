#pragma once

#include "resource.h"
#include "util/rect.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

enum class CompareFunc {
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always,
};

class RenderState : public Resource {
	RTTR_ENABLE(Resource)
public:
	struct Viewport {
		util::RectF _rect;
		float _minDepth, _maxDepth;

		size_t GetHash() const;
		bool operator==(Viewport const &other) const;
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

	struct CullState {
		FrontFaceMode _front = FrontFaceMode::CCW;
		CullMask _cullMask = CullMask::None;

		size_t GetHash() const;
		bool operator==(CullState const &other) const;
	};

	struct DepthBias {
		bool _enable = false;
		float _constantFactor = 0;
		float _clamp = 0;
		float _slopeFactor = 0;

		size_t GetHash() const;
		bool operator==(DepthBias const &other) const;
	};

	struct DepthState {
		bool _depthTestEnable = false;
		bool _depthWriteEnable = false;
		CompareFunc _depthCompareFunc = CompareFunc::Less;
		bool _depthBoundsTestEnable = false;
		float _minDepthBounds = 0.0f;
		float _maxDepthBounds = 1.0f;

		size_t GetHash() const;
		bool operator==(DepthState const &other) const;
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

	struct StencilFuncState {
		StencilFunc _failFunc = StencilFunc::Keep;
		StencilFunc _passFunc = StencilFunc::Keep;
		StencilFunc _depthFailFunc = StencilFunc::Keep;
		CompareFunc _compareFunc = CompareFunc::Never;
		uint32_t _compareMask = 0;
		uint32_t _writeMask = 0;
		uint32_t _reference = 0;

		size_t GetHash() const;
		bool operator==(StencilFuncState const &other) const;
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

	struct BlendFuncState {
		bool _blendEnable = false;
		BlendFactor _srcColorBlendFactor = BlendFactor::SrcAlpha;
		BlendFactor _dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
		BlendFunc _colorBlendFunc = BlendFunc::Add;
		BlendFactor _srcAlphaBlendFactor = BlendFactor::SrcAlpha;
		BlendFactor _dstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha;
		BlendFunc _alphaBlendFunc = BlendFunc::Add;
		ColorComponentMask _colorWriteMask = ColorComponentMask::RGBA;

		size_t GetHash() const;
		bool operator==(BlendFuncState const &other) const;
	};

	struct StateData {
		Viewport _viewport;
		util::RectI _scissor;
		CullState _cullState;
		DepthBias _depthBias;
		DepthState _depthState;
		bool _stencilEnable = false;
		std::array<StencilFuncState, 2> _stencilState;
		std::vector<BlendFuncState> _blendStates{ BlendFuncState() };
		glm::vec4 _blendColor{};

		size_t GetHash() const;
		bool operator==(StateData const &other) const;
	};

	RenderState(Device &device);

	virtual void Init() = 0;

	virtual void SetViewport(util::RectF rect, float minDepth, float maxDepth);
	virtual Viewport const &GetViewport() { return _data._viewport; }

	virtual void SetScissor(util::RectI rect);
	virtual util::RectI const &GetScissor() { return _data._scissor; }

	virtual void SetCullState(FrontFaceMode front, CullMask cull);
	virtual CullState const & GetCullState() { return _data._cullState; }

	virtual void SetDepthBias(bool enable, float constantFactor = 0, float clamp = 0, float slopeFactor = 0);
	virtual DepthBias const &GetDepthBias() { return _data._depthBias; }

	virtual void SetDepthState(bool testEnable, bool writeEnable, CompareFunc compareFunc = CompareFunc::Less, bool boundsTestEnable = false, float minBounds = 0, float maxBounds = 0);
	virtual DepthState const &GetDepthState() { return _data._depthState; }

	virtual void SetStencilEnable(bool enable);
	virtual bool GetStencilEnable() { return _data._stencilEnable; }

	virtual void SetFrontStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
	virtual StencilFuncState const &GetFrontStencil() { return _data._stencilState[0]; }

	virtual void SetBackStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
	virtual StencilFuncState const &GetBackStencil() { return _data._stencilState[1]; }

	virtual uint32_t GetAttachmentBlendStateCount() { return static_cast<uint32_t>(_data._blendStates.size()); }
	virtual void SetAttachmentBlendState(uint32_t attachmentIndex, bool blendEnable,
		BlendFactor srcColorFactor, BlendFactor dstColorFactor, BlendFunc colorBlendFunc,
		BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor, BlendFunc alphaBlendFunc,
		ColorComponentMask colorWriteMask);
	virtual BlendFuncState const &GetAttachmentBlendState(uint32_t attachmentIndex) { return _data._blendStates[attachmentIndex]; }

	virtual void SetBlendConstant(glm::vec4 color);
	virtual glm::vec4 const &GetBlendConstant() { return _data._blendColor; }

	StateData const &GetStateData() { return _data; }
	uint32_t GetStateDataVersion() const { return _dataVersion; }
public:
	void SetStencil(StencilFuncState &stencil, StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference);

	StateData _data;
	uint32_t _dataVersion = 1;
};

DEFINE_ENUM_BIT_OPERATORS(RenderState::ColorComponentMask)

NAMESPACE_END(gr1)

NAMESPACE_BEGIN(std)

template<>
struct hash<gr1::RenderState::StencilFuncState> { size_t operator()(gr1::RenderState::StencilFuncState const &s) { return s.GetHash(); } };

template<>
struct hash<gr1::RenderState::BlendFuncState> { size_t operator()(gr1::RenderState::BlendFuncState const &b) { return b.GetHash(); } };


NAMESPACE_END(std)