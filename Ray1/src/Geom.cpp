#include "stdafx.h"

#include "Geom.h"

bool LineBoxIntersect(Line3f const & line, AABB const & box, float & t0, float & t1)
{
  assert(!line.direction().isZero());

  t0 = std::numeric_limits<float>::infinity();
  t1 = -std::numeric_limits<float>::infinity();

  for (int dim = 0; dim < 3; ++dim) {
    if (Eq(line.direction()(dim), 0)) {
      if (box.min()(dim) >= line.origin()(dim) || line.origin()(dim) <= box.max()(dim)) {
        // line's outside the box
        t0 = std::numeric_limits<float>::infinity();
        t1 = -std::numeric_limits<float>::infinity();
        return false;
      }
    } else {
      float tMin = (box.min()(dim) - line.origin()(dim)) / line.direction()(dim);
      float tMax = (box.max()(dim) - line.origin()(dim)) / line.direction()(dim);
      if (line.direction()(dim) < 0) {
        t0 = std::max(t0, tMax);
        t1 = std::min(t1, tMin);
      } else {
        t0 = std::max(t0, tMin);
        t1 = std::min(t1, tMax);
      }
    }
  }

  return t0 <= t1;
}

bool LineSphereIntersect(Line3f const & line, Sphere const & sphere, float & t0, float & t1)
{
  assert(sphere.m_Radius >= 0);

  Vector3f m = line.origin() - sphere.m_Center;
  float a = Dot(line.direction(), line.direction());
  float b = 2 * Dot(line.direction(), m);
  float c = Dot(m, m) - Sqr(sphere.m_Radius);

  int roots = SolveQuadratic(a, b, c, t0, t1);

  return roots > 0;
}
