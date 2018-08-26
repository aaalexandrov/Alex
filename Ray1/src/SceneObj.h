#pragma once

#include "Octree.h"
#include "SceneMaterial.h"
#include "Ray.h"

class SceneObj : public Bounded {
public:
  virtual bool Intersect(Ray &ray) const = 0;

public:
  std::shared_ptr<SceneMaterial> m_Material;
  Affine3f m_Transform;
};