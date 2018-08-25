#pragma once

#include <stdint.h>
#include "../Eigen/Dense"

#undef near
#undef far

uint32_t NextPowerOf2(uint32_t v)
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

Eigen::Matrix4f Ortho(float left, float right, float top, float bottom, float near, float far)
{
  Eigen::Matrix4f m;
  m << 2 / (right - left), 0, 0, -(right + left) / (right - left),
    0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
    0, 0, -2 / (far - near), -(far + near) / (far - near),
    0, 0, 0, 1;
  return m;
}
