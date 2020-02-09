#include "render_state.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr1)

size_t RenderState::Viewport::GetHash() const
{
	size_t hash = util::GetHash(_rect);
	hash = util::GetHash(_minDepth, hash);
	hash = util::GetHash(_maxDepth, hash);
	return hash;
}

bool RenderState::Viewport::operator==(Viewport const &other) const
{
	return _rect == other._rect && _minDepth == other._minDepth && _maxDepth == other._maxDepth;
}

size_t RenderState::CullState::GetHash() const
{
	size_t hash = util::GetHash(_front);
	hash = util::GetHash(_cullMask, hash);
	return hash;
}

bool RenderState::CullState::operator==(CullState const &other) const
{
	return _front == other._front && _cullMask == other._cullMask;
}

size_t RenderState::DepthBias::GetHash() const
{
	size_t hash = util::GetHash(_enable);
	hash = util::GetHash(_constantFactor, hash);
	hash = util::GetHash(_clamp, hash);
	hash = util::GetHash(_slopeFactor, hash);
	return hash;
}

bool RenderState::DepthBias::operator==(DepthBias const &other) const
{
	return _enable == other._enable
		&& _constantFactor == other._constantFactor
		&& _clamp == other._clamp
		&& _slopeFactor == other._slopeFactor;
}

size_t RenderState::DepthState::GetHash() const
{
	size_t hash = util::GetHash(_depthTestEnable);
	hash = util::GetHash(_depthWriteEnable, hash);
	hash = util::GetHash(_depthCompareFunc, hash);
	hash = util::GetHash(_depthBoundsTestEnable, hash);
	hash = util::GetHash(_minDepthBounds, hash);
	hash = util::GetHash(_maxDepthBounds, hash);
	return hash;
}

bool RenderState::DepthState::operator==(DepthState const &other) const
{
	return _depthTestEnable == other._depthTestEnable
		&& _depthWriteEnable == other._depthWriteEnable
		&& _depthCompareFunc == other._depthCompareFunc
		&& _depthBoundsTestEnable == other._depthBoundsTestEnable
		&& _minDepthBounds == other._minDepthBounds
		&& _maxDepthBounds == other._maxDepthBounds;
}

size_t RenderState::StencilFuncState::GetHash() const
{
	size_t hash = util::GetHash(_failFunc);
	hash = util::GetHash(_passFunc, hash);
	hash = util::GetHash(_depthFailFunc, hash);
	hash = util::GetHash(_compareFunc, hash);
	hash = util::GetHash(_compareMask, hash);
	hash = util::GetHash(_writeMask, hash);
	hash = util::GetHash(_reference, hash);
	return hash;
}

bool RenderState::StencilFuncState::operator==(StencilFuncState const &other) const
{
	return _failFunc == other._failFunc
		&& _passFunc == other._passFunc
		&& _depthFailFunc == other._depthFailFunc
		&& _compareFunc == other._compareFunc
		&& _compareMask == other._compareMask
		&& _writeMask == other._writeMask
		&& _reference == other._reference;
}

size_t RenderState::BlendFuncState::GetHash() const
{
	size_t hash = util::GetHash(_blendEnable);
	hash = util::GetHash(_srcColorBlendFactor, hash);
	hash = util::GetHash(_dstColorBlendFactor, hash);
	hash = util::GetHash(_colorBlendFunc, hash);
	hash = util::GetHash(_srcAlphaBlendFactor, hash);
	hash = util::GetHash(_dstAlphaBlendFactor, hash);
	hash = util::GetHash(_alphaBlendFunc, hash);
	hash = util::GetHash(_colorWriteMask, hash);
	return hash;
}

bool RenderState::BlendFuncState::operator==(BlendFuncState const &other) const
{
	return _blendEnable == other._blendEnable
		&& _srcColorBlendFactor == other._srcColorBlendFactor
		&& _dstColorBlendFactor == other._dstColorBlendFactor
		&& _colorBlendFunc == other._colorBlendFunc
		&& _srcAlphaBlendFactor == other._srcAlphaBlendFactor
		&& _dstAlphaBlendFactor == other._dstAlphaBlendFactor
		&& _alphaBlendFunc == other._alphaBlendFunc
		&& _colorWriteMask == other._colorWriteMask;
}

size_t RenderState::StateData::GetHash() const
{
	size_t hash = _viewport.GetHash();
	hash = util::GetHash(_scissor, hash);
	hash = 31 * hash + _cullState.GetHash();
	hash = 31 * hash + _depthBias.GetHash();
	hash = 31 * hash + _depthState.GetHash();
	hash = util::GetHash(_stencilEnable, hash);
	hash = util::GetHash(_stencilState, hash);
	hash = util::GetHash(_blendStates, hash);
	hash = util::GetHash(_blendColor, hash);
	return hash;
}

bool RenderState::StateData::operator==(StateData const &other) const
{
	return _viewport == other._viewport
		&& _scissor == other._scissor
		&& _cullState == other._cullState
		&& _depthBias == other._depthBias
		&& _depthState == other._depthState
		&& _stencilEnable == other._stencilEnable
		&& _stencilState == other._stencilState
		&& _blendStates == other._blendStates
		&& _blendColor == other._blendColor;
}


RenderState::RenderState(Device &device) 
	: Resource(device)
{

}

void RenderState::SetViewport(util::RectF rect, float minDepth, float maxDepth)
{
	_data._viewport._rect = rect;
	_data._viewport._minDepth = minDepth;
	_data._viewport._maxDepth = maxDepth;
}

void RenderState::SetScissor(util::RectI rect)
{
	_data._scissor = rect;
}

void RenderState::SetCullState(FrontFaceMode front, CullMask cull)
{
	_data._cullState._front = front;
	_data._cullState._cullMask = cull;
}

void RenderState::SetDepthBias(bool enable, float constantFactor, float clamp, float slopeFactor)
{
	_data._depthBias._enable = enable;
	_data._depthBias._constantFactor = constantFactor;
	_data._depthBias._clamp = clamp;
	_data._depthBias._slopeFactor = slopeFactor;
}

void RenderState::SetDepthState(bool testEnable, bool writeEnable, CompareFunc compareFunc, bool boundsTestEnable, float minBounds, float maxBounds)
{
	_data._depthState._depthTestEnable = testEnable;
	_data._depthState._depthWriteEnable = writeEnable;
	_data._depthState._depthCompareFunc = compareFunc;
	_data._depthState._depthBoundsTestEnable = boundsTestEnable;
	_data._depthState._minDepthBounds = minBounds;
	_data._depthState._maxDepthBounds = maxBounds;
}

void RenderState::SetStencilEnable(bool enable)
{
	_data._stencilEnable = enable;
}

void RenderState::SetFrontStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
{
	SetStencil(_data._stencilState[0], failFunc, passFunc, depthFailFunc, compareFunc, compareMask, writeMask, reference);
}

void RenderState::SetBackStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
{
	SetStencil(_data._stencilState[1], failFunc, passFunc, depthFailFunc, compareFunc, compareMask, writeMask, reference);
}

void RenderState::SetAttachmentBlendState(uint32_t attachmentIndex, bool blendEnable, BlendFactor srcColorFactor, BlendFactor dstColorFactor, BlendFunc colorBlendFunc, BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor, BlendFunc alphaBlendFunc, ColorComponentMask colorWriteMask)
{
	if (attachmentIndex >= _data._blendStates.size())
		_data._blendStates.resize(attachmentIndex + 1);
	_data._blendStates[attachmentIndex]._blendEnable = blendEnable;
	_data._blendStates[attachmentIndex]._srcColorBlendFactor = srcColorFactor;
	_data._blendStates[attachmentIndex]._dstColorBlendFactor = dstColorFactor;
	_data._blendStates[attachmentIndex]._colorBlendFunc = colorBlendFunc;
	_data._blendStates[attachmentIndex]._srcAlphaBlendFactor = srcAlphaFactor;
	_data._blendStates[attachmentIndex]._dstAlphaBlendFactor = dstAlphaFactor;
	_data._blendStates[attachmentIndex]._alphaBlendFunc = alphaBlendFunc;
	_data._blendStates[attachmentIndex]._colorWriteMask = colorWriteMask;
}

void RenderState::SetBlendConstant(glm::vec4 color)
{
	_data._blendColor = color;
}

void RenderState::SetStencil(StencilFuncState &stencil, StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
{
	stencil._failFunc = failFunc;
	stencil._passFunc = passFunc;
	stencil._depthFailFunc = depthFailFunc;
	stencil._compareFunc = compareFunc;
	stencil._compareMask = compareMask;
	stencil._writeMask = writeMask;
	stencil._reference = reference;
}

NAMESPACE_END(gr1)

