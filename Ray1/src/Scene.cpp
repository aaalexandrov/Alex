#include "stdafx.h"
#include "Scene.h"

Scene::Scene(AABB const & bound, float minSize):
  m_Octree(bound, minSize)
{
}

void Scene::SetCamera(std::shared_ptr<Camera> &camera)
{
  m_Camera = camera;
}

void Scene::Add(std::shared_ptr<SceneObj> &obj)
{
  m_Objects.insert(obj);
  m_Octree.Add(*obj);
}

void Scene::Remove(std::shared_ptr<SceneObj> &obj)
{
  m_Octree.Remove(*obj);
  m_Objects.erase(obj);
}
