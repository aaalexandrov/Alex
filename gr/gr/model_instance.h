#pragma once

#include "graphics_resource.h"
#include "model.h"

NAMESPACE_BEGIN(gr)

class ModelInstance : public GraphicsResource {
public:

  Model *GetModel() { return _model.get(); }
  void SetModel(Model *model);

  virtual void Invalidate() {}

  glm::mat4x4 _transform;
  std::shared_ptr<Model> _model;
  std::vector<std::shared_ptr<Buffer>> _uniforms;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::ModelInstance, gr::GraphicsResource)