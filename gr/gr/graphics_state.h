#pragma once

#include "util/namespace.h"

NAMESPACE_BEGIN(gr)

enum class FillMode {
  Solid,
  Wireframe,
};

enum class BlendMode {
  Opaque,
  Transparent,
};

enum class CullMode {
  None,
  Back,
  Front,
};

enum class DepthTest {
  None,
  Less,
};

class GraphicsState {
public:
  virtual ~GraphicsState() {}

  FillMode GetFillMode() { return _fillMode; }
  void SetFillMode(FillMode fillMode);

  BlendMode GetBlendMode() { return _blendMode; }
  void SetBlendMode(BlendMode blendMode);

  CullMode GetCullMode() { return _cullMode; }
  void SetCullMode(CullMode cullMode);

  DepthTest GetDepthTest() { return _depthTest; }
  void SetDepthTest(DepthTest depthTest);

  bool GetDepthWrite() { return _depthWrite; }
  void SetDepthWrite(bool depthWrite);

  virtual void Invalidate() {}

  FillMode _fillMode = FillMode::Solid;
  BlendMode _blendMode = BlendMode::Opaque;
  CullMode _cullMode = CullMode::Back;
  DepthTest _depthTest = DepthTest::Less;
  bool _depthWrite = true;
};


NAMESPACE_END(gr)