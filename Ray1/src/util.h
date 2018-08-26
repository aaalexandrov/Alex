#pragma once

#include <stdint.h>
#include <limits>
#include "Eigen/Dense"
#include "Eigen/Geometry"

#undef near
#undef far

typedef Eigen::Vector3f Vector3f;
typedef Eigen::Vector4f Vector4f;
typedef Eigen::Matrix4f Matrix4f;
typedef Eigen::Affine3f Affine3f;
typedef Eigen::Projective3f Projective3f;
typedef Eigen::AlignedBox3f AABB;
typedef Eigen::ParametrizedLine<float, 3> Line3f;

inline uint32_t NextPowerOf2(uint32_t v)
{
  --v;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  ++v;

  return v;
}

inline Matrix4f Orthographic(float left, float right, float top, float bottom, float near, float far)
{
  Matrix4f m;
  m << 2 / (right - left), 0, 0, -(right + left) / (right - left),
    0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
    0, 0, -2 / (far - near), -(far + near) / (far - near),
    0, 0, 0, 1;
  return m;
}

// left, right, top, bottom are coordinates of the frustum vertices on the near plane
inline Matrix4f Projection(float left, float right, float top, float bottom, float near, float far)
{
  Matrix4f m;
  m << 2 * near / (right - left), 0, (right + left) / (right - left), 0,
    0, 2 * near / (top - bottom), (top + bottom) / (top - bottom), 0,
    0, 0, -(far + near) / (far - near), -2 * far * near / (far - near),
    0, 0, -1, 0;
  return m;
}

inline Matrix4f Projection(float horFov, float ratio, float near, float far)
{
  float w2 = std::tan(horFov / 2) * near;
  float h2 = w2 / ratio;
  return Projection(-w2, w2, -h2, h2, near, far);
}

inline bool Eq(float x, float y, float eps = std::numeric_limits<float>::epsilon())
{
  return std::abs(x - y) <= eps;
}

template <class T>
T Sqr(T a)
{
  return a * a;
}

inline float Dot(Vector3f const &a, Vector3f const &b)
{
  return a.dot(b);
}

inline Vector3f Cross(Vector3f const &a, Vector3f const &b)
{
  return a.cross(b);
}

int SolveQuadratic(float a, float b, float c, float &x0, float &x1);

template <class T>
void RemoveFromVector(std::vector<T> &vec, T const &value)
{
  vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
}