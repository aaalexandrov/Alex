#pragma once

#include "resource.h"
#include "util/rect.h"
#include "util/enumutl.h"

NAMESPACE_BEGIN(gr1)

class RenderState : public Resource {
	RTTR_ENABLE(Resource)
public:
	struct Viewport {
		util::RectF _rect;
		float _minDepth, _maxDepth;
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
	};

	struct DepthBias {
		bool _enable = false;
		float _constantFactor = 0;
		float _clamp = 0;
		float _slopeFactor = 0;
	};

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

	struct DepthState {
		bool _depthTestEnable = false;
		bool _depthWriteEnable = false;
		CompareFunc _depthCompareFunc = CompareFunc::Never;
		bool _depthBoundsTestEnable = false;
		float _minDepthBounds = 0;
		float _maxDepthBounds = 0;
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
	};

	struct BlendFuncState {
		bool _blendEnable = false;
		BlendFactor _srcColorBlendFactor = BlendFactor::Zero;
		BlendFactor _dstColorBlendFactor = BlendFactor::Zero;
		BlendFunc _colorBlendFunc = BlendFunc::Add;
		BlendFactor _srcAlphaBlendFactor = BlendFactor::Zero;
		BlendFactor _dstAlphaBlendFactor = BlendFactor::Zero;
		BlendFunc _alphaBlendFunc = BlendFunc::Add;
		ColorComponentMask _colorWriteMask = ColorComponentMask::None;
	};

	RenderState(Device &device);

	virtual void Init() = 0;

	virtual void SetViewport(util::RectF rect, float minDepth, float maxDepth);
	virtual Viewport const &GetViewport() { return _viewport; }

	virtual void SetScissor(util::RectI rect);
	virtual util::RectI const &GetScissor() { return _scissor; }

	virtual void SetCullState(FrontFaceMode front, CullMask cull);
	virtual CullState const & GetCullState() { return _cullState; }

	virtual void SetDepthBias(bool enable, float constantFactor = 0, float clamp = 0, float slopeFactor = 0);
	virtual DepthBias const &GetDepthBias() { return _depthBias; }

	virtual void SetDepthState(bool testEnable, bool writeEnable, CompareFunc compareFunc = CompareFunc::Less, bool boundsTestEnable = false, float minBounds = 0, float maxBounds = 0);
	virtual DepthState const &GetDepthState() { return _depthState; }

	virtual void SetStencilEnable(bool enable);
	virtual bool GetStencilEnable() { return _stencilEnable; }

	virtual void SetFrontStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
	virtual StencilFuncState const &GetFrontStencil() { return _stencilState[0]; }

	virtual void SetBackStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference);
	virtual StencilFuncState const &GetBackStencil() { return _stencilState[1]; }

	virtual uint32_t GetAttachmentBlendStateCount() { return static_cast<uint32_t>(_blendStates.size()); }
	virtual void SetAttachmentBlendState(uint32_t attachmentIndex, bool blendEnable,
		BlendFactor srcColorFactor, BlendFactor dstColorFactor, BlendFunc colorBlendFunc,
		BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor, BlendFunc alphaBlendFunc,
		ColorComponentMask colorWriteMask);
	virtual BlendFuncState const &GetAttachmentBlendState(uint32_t attachmentIndex) { return _blendStates[attachmentIndex]; }

	virtual void SetBlendConstant(glm::vec4 color);
	virtual glm::vec4 const &GetBlendConstant() { return _blendColor; }
public:
	void SetStencil(StencilFuncState &stencil, StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference);

	Viewport _viewport;
	util::RectI _scissor;
	CullState _cullState;
	DepthBias _depthBias;
	DepthState _depthState;
	bool _stencilEnable = false;
	std::array<StencilFuncState, 2> _stencilState;
	std::vector<BlendFuncState> _blendStates;
	glm::vec4 _blendColor;
};

DEFINE_ENUM_BIT_OPERATORS(RenderState::ColorComponentMask)

NAMESPACE_END(gr1)

