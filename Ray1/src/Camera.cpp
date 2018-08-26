#include "stdafx.h"
#include "Camera.h"

Camera::Camera(std::shared_ptr<Backbuffer>& backbuffer) :
  m_Backbuffer(backbuffer)
{
  SetProjection(M_PI, backbuffer->m_Image.cols() / (float)backbuffer->m_Image.rows(), 0.1f, 1000.0f);
}

void Camera::SetProjection(float horFov, float ratio, float near, float far)
{
  m_Proj = Projection(horFov, ratio, near, far);
  UpdateViewProj();
}

void Camera::SetTransform(Affine3f const & transform)
{
  m_Transform = transform;
  UpdateViewProj();
}

void Camera::CalculateRay(uint32_t x, uint32_t y, Ray & ray) const
{
  Vector4f p;
  p(0) = 2 * (x + 0.5f) / GetWidth() - 1;
  p(1) = 1 - 2 * (y + 0.5f) / GetHeight();
  p(2) = -1;
  p(3) = 1;
  Vector4f w = m_ViewProjInv * p;
  ray.m_Ray = Line3f::Through(m_Transform.translation(), (w / w(3)).head<3>());
}

void Camera::UpdateViewProj()
{
  m_ViewProj = m_Proj * m_Transform.inverse();
  m_ViewProjInv = m_ViewProj.inverse();
}
