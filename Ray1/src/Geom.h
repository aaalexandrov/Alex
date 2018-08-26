#pragma once

#include "Util.h"

class Sphere {
public:
  Sphere(Vector3f const &center, float radius) : m_Center(center), m_Radius(radius) {}
public:
  Vector3f m_Center;
  float m_Radius;
};

bool LineBoxIntersect(Line3f const &line, AABB const &box, float &t0, float &t1);
bool LineSphereIntersect(Line3f const &line, Sphere const &sphere, float &t0, float &t1);