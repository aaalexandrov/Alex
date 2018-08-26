#pragma once

#include "Geom.h"
#include "Backbuffer.h"
#include "Ray.h"

class Camera {
public:
  Camera(std::shared_ptr<Backbuffer> &backbuffer);

  void SetProjection(float horFov, float ratio, float near, float far);
  void SetTransform(Affine3f const &transform);

  uint32_t GetWidth()  const { return m_Backbuffer->m_Image.cols(); }
  uint32_t GetHeight() const { return m_Backbuffer->m_Image.rows(); }

  void CalculateRay(uint32_t x, uint32_t y, Ray &ray) const;

public:
  std::shared_ptr<Backbuffer> m_Backbuffer;
  Projective3f m_Proj, m_ViewProj, m_ViewProjInv;
  Affine3f m_Transform = Affine3f::Identity();

  void UpdateViewProj();
};