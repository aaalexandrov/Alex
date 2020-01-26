#include "render_state.h"

NAMESPACE_BEGIN(gr1)

RenderState::RenderState(Device &device) 
	: Resource(device)
{

}

void RenderState::SetViewport(util::RectF rect, float minDepth, float maxDepth)
{
	_viewport._rect = rect;
	_viewport._minDepth = minDepth;
	_viewport._maxDepth = maxDepth;
}

void RenderState::SetScissor(util::RectI rect)
{
	_scissor = rect;
}

void RenderState::SetCullState(FrontFaceMode front, CullMask cull)
{
	_cullState._front = front;
	_cullState._cullMask = cull;
}

void RenderState::SetDepthBias(bool enable, float constantFactor, float clamp, float slopeFactor)
{
	_depthBias._enable = enable;
	_depthBias._constantFactor = constantFactor;
	_depthBias._clamp = clamp;
	_depthBias._slopeFactor = slopeFactor;
}

void RenderState::SetDepthState(bool testEnable, bool writeEnable, CompareFunc compareFunc, bool boundsTestEnable, float minBounds, float maxBounds)
{
	_depthState._depthTestEnable = testEnable;
	_depthState._depthWriteEnable = writeEnable;
	_depthState._depthCompareFunc = compareFunc;
	_depthState._depthBoundsTestEnable = boundsTestEnable;
	_depthState._minDepthBounds = minBounds;
	_depthState._maxDepthBounds = maxBounds;
}

void RenderState::SetStencilEnable(bool enable)
{
	_stencilEnable = enable;
}

void RenderState::SetFrontStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
{
	SetStencil(_stencilState[0], failFunc, passFunc, depthFailFunc, compareFunc, compareMask, writeMask, reference);
}

void RenderState::SetBackStencil(StencilFunc failFunc, StencilFunc passFunc, StencilFunc depthFailFunc, CompareFunc compareFunc, uint32_t compareMask, uint32_t writeMask, uint32_t reference)
{
	SetStencil(_stencilState[1], failFunc, passFunc, depthFailFunc, compareFunc, compareMask, writeMask, reference);
}

void RenderState::SetAttachmentBlendState(uint32_t attachmentIndex, bool blendEnable, BlendFactor srcColorFactor, BlendFactor dstColorFactor, BlendFunc colorBlendFunc, BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor, BlendFunc alphaBlendFunc, ColorComponentMask colorWriteMask)
{
	if (attachmentIndex >= _blendStates.size())
		_blendStates.resize(attachmentIndex + 1);
	_blendStates[attachmentIndex]._blendEnable = blendEnable;
	_blendStates[attachmentIndex]._srcColorBlendFactor = srcColorFactor;
	_blendStates[attachmentIndex]._dstColorBlendFactor = dstColorFactor;
	_blendStates[attachmentIndex]._colorBlendFunc = colorBlendFunc;
	_blendStates[attachmentIndex]._srcAlphaBlendFactor = srcAlphaFactor;
	_blendStates[attachmentIndex]._dstAlphaBlendFactor = dstAlphaFactor;
	_blendStates[attachmentIndex]._alphaBlendFunc = alphaBlendFunc;
	_blendStates[attachmentIndex]._colorWriteMask = colorWriteMask;
}

void RenderState::SetBlendConstant(glm::vec4 color)
{
	_blendColor = color;
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

