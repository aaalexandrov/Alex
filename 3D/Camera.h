#ifndef __CAMERA_H
#define __CAMERA_H

#include "Transform.h"
#include "Convex.h"

class CCamera {
public:
  CXForm m_XForm;
  CMatrix<4, 4> m_mProjection;
  CMatrix<4, 4> m_mView;
  float m_fWHRatio, m_fHorizontalFOV, m_fNear, m_fFar;
  CConvex m_Frustum;

  CCamera();
  ~CCamera();

  bool Init(float fWHRatio, float fHorizontalFOV, float fNear, float fFar, CMatrix<4, 4> *pMatXForm);

  void Update();
  void UpdateFrustum();

  void SetVars(CVarObj *pTargetVars = 0);
  void UnsetVars(CVarObj *pTargetVars = 0);
};

#endif