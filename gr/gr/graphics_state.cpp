#include "graphics_state.h"

NAMESPACE_BEGIN(gr)

void GraphicsState::SetFillMode(FillMode fillMode)
{
  _fillMode = fillMode;
  Invalidate();
}

void GraphicsState::SetBlendMode(BlendMode blendMode)
{
  _blendMode = blendMode;
  Invalidate();
}

void GraphicsState::SetCullMode(CullMode cullMode)
{
  _cullMode = cullMode;
  Invalidate();
}

void GraphicsState::SetDepthTest(DepthTest depthTest)
{
  _depthTest = depthTest;
  Invalidate();
}

void GraphicsState::SetDepthWrite(bool depthWrite)
{
  _depthWrite = depthWrite;
  Invalidate();
}

void GraphicsState::Invalidate()
{
}

NAMESPACE_END(gr)

