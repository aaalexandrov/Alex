#pragma once

#include "util/namespace.h"

NAMESPACE_BEGIN(gr)

class PresentationSurface;
class ResourceUpdate;
class ModelInstance;

class RenderQueue {
public:
  std::shared_ptr<PresentationSurface> GetPresentationSurface() { return _presentationSurface; }
  void SetPresentationSurface(std::shared_ptr<PresentationSurface> &presentationSurface) { _presentationSurface = presentationSurface; }

  virtual void AddModelInstance(std::shared_ptr<ModelInstance> &modelInst) { _instancesToRender.push_back(modelInst); }
  virtual void AddResourceUpdate(std::shared_ptr<ResourceUpdate> &resouceUpdate);

  virtual void Render() = 0;
  virtual void ProcessResourceUpdates();

  std::shared_ptr<PresentationSurface> _presentationSurface;

  std::vector<std::shared_ptr<ModelInstance>> _instancesToRender;
  std::vector<std::shared_ptr<ResourceUpdate>> _resourceUpdates;
};

NAMESPACE_END(gr)