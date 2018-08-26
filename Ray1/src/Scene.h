#pragma once

#include "SceneObj.h"
#include "Camera.h"

#include <set>

class Scene {
public:
  Scene(AABB const &bound, float minSize);

  void SetCamera(std::shared_ptr<Camera> &camera);

  void Add(std::shared_ptr<SceneObj> &obj);
  void Remove(std::shared_ptr<SceneObj> &obj);

public:
  std::shared_ptr<Camera> m_Camera;

  std::set<std::shared_ptr<SceneObj>> m_Objects;
  Octree m_Octree;
};