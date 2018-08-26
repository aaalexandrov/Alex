#pragma once

#include "Geom.h"
#include "SceneMaterial.h"

class RayHit {
public:
  float m_HitDistance;
  Vector3f m_HitPoint;
  Vector3f m_HitNormal;
  SceneMaterial *m_HitMaterial;
};

class Ray {
public:
  Line3f m_Ray;
  RayHit m_Hit;
};