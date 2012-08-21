#include "stdafx.h"
#include "Camera.h"
#include "Graphics.h"

CCamera::CCamera()
{
}

CCamera::~CCamera()
{
  UnsetVars();
}

bool CCamera::Init(float fWHRatio, float fHorizontalFOV, float fNear, float fFar, CMatrix<4, 4> *pMatXForm)
{
  m_fWHRatio = fWHRatio;
  m_fHorizontalFOV = fHorizontalFOV;
  m_fNear = fNear;
  m_fFar = fFar;
  m_mProjection.SetProjection(fWHRatio, fHorizontalFOV, fNear, fFar);
//  m_mProjection.SetOrthographic(10, 10 / fWHRatio, fNear, fFar);
  if (pMatXForm)
    *m_XForm.m_pMatrix = *pMatXForm;
  else
    m_XForm.m_pMatrix->SetDiagonal();

  Update();

  return true;
}

void CCamera::Update()
{
  m_XForm.OrthonormalizeRotation();
  m_XForm.m_pMatrix->GetInverse(m_mView);
  UpdateFrustum();
}

void CCamera::UpdateFrustum()
{
  CVector<3> vPoints[2][2][2];
  CVector<4> v, vPlane;
  int x, y, z;

  m_Frustum.Reset();

  for (x = 0; x < 2; x++)
    for (y = 0; y < 2; y++)
      for (z = 0; z < 2; z++) {
        v.z() = z ? m_fFar : m_fNear;
        v.x() = v.z() * tan(m_fHorizontalFOV / 2);
        v.y() = v.x() / m_fWHRatio * (y * 2 - 1);
        v.x() = v.x() * (x * 2 - 1);
        v.w() = 1;

        v = v * (*m_XForm.m_pMatrix);

        vPoints[x][y][z].Set(v);
        vPoints[x][y][z] /= v.w();
      }

#ifdef _DEBUG
  CVector<3> vMid;
  v.z() = (m_fNear + m_fFar) / 2;
  v.x() = v.y() = 0;
  v.w() = 1;
  v = v * (*m_XForm.m_pMatrix);
  vMid.Set(v);
  vMid /= v.w();
#endif

  // Near
  vPlane = CPlane::CalcFromPoints(vPoints[0][0][0], vPoints[0][1][0], vPoints[1][0][0]);
  ASSERT(CPlane::CalcValue(vPlane, vMid) < 0);
  m_Frustum.AddPlane(vPlane);

  // Far
  vPlane = CPlane::CalcFromPoints(vPoints[1][1][1], vPoints[0][1][1], vPoints[1][0][1]);
  ASSERT(CPlane::CalcValue(vPlane, vMid) < 0);
  m_Frustum.AddPlane(vPlane);

  // Left
  vPlane = CPlane::CalcFromPoints(vPoints[0][0][0], vPoints[0][0][1], vPoints[0][1][0]);
  ASSERT(CPlane::CalcValue(vPlane, vMid) < 0);
  m_Frustum.AddPlane(vPlane);

  // Right
  vPlane = CPlane::CalcFromPoints(vPoints[1][1][1], vPoints[1][0][1], vPoints[1][1][0]);
  ASSERT(CPlane::CalcValue(vPlane, vMid) < 0);
  m_Frustum.AddPlane(vPlane);

  // Top
  vPlane = CPlane::CalcFromPoints(vPoints[0][0][0], vPoints[1][0][0], vPoints[0][0][1]);
  ASSERT(CPlane::CalcValue(vPlane, vMid) < 0);
  m_Frustum.AddPlane(vPlane);

  // Bottom
  vPlane = CPlane::CalcFromPoints(vPoints[1][1][1], vPoints[1][1][0], vPoints[0][1][1]);
  ASSERT(CPlane::CalcValue(vPlane, vMid) < 0);
  m_Frustum.AddPlane(vPlane);

  m_Frustum.InitData();
}

void CCamera::SetVars(CVarObj *pTargetVars)
{
  static CStrAny sg_mView(ST_CONST, "g_mView");
  static CStrAny sg_mProj(ST_CONST, "g_mProj");

  if (!pTargetVars)
    pTargetVars = &CGraphics::Get()->m_Globals;

  CMatrixVar *pViewVar = new CMatrixVar(m_mView.GetRows(), m_mView.GetCols(), 0, &m_mView(0, 0));
  pTargetVars->ReplaceVar(sg_mView, pViewVar);

  CMatrixVar *pProjVar = new CMatrixVar(m_mProjection.GetRows(), m_mProjection.GetCols(), 0, &m_mProjection(0, 0));
  pTargetVars->ReplaceVar(sg_mProj, pProjVar);
}

void CCamera::UnsetVars(CVarObj *pTargetVars)
{
  static CStrAny sg_mView(ST_CONST, "g_mView");
  static CStrAny sg_mProj(ST_CONST, "g_mProj");

  if (!pTargetVars)
    pTargetVars = &CGraphics::Get()->m_Globals;

  pTargetVars->ReplaceVar(sg_mView, 0);
  pTargetVars->ReplaceVar(sg_mProj, 0);
}
