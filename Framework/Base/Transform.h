#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include "Matrix.h"
#include "Vector.h"

class CXForm {
public:
  CMatrix<4, 4> *m_pMatrix;

  inline CXForm(CMatrix<4, 4> *pMatrix = 0)  { if (pMatrix) m_pMatrix = pMatrix; else { m_pMatrix = new CMatrix<4, 4>(); SetIdentity(); } }
  inline ~CXForm() { delete m_pMatrix; }

  inline CVector<3> GetTranslation() const;
  inline CVector<3> GetScale() const;
  inline CMatrix<3, 3> GetRotation() const;

  inline void SetTranslation(const CVector<3> &vTrans);
  inline void SetScale(const CVector<3> &vScale);
  inline void SetRotation(const CMatrix<3, 3> &mRotation);

  inline void SetIdentity() { m_pMatrix->SetDiagonal(); }

  inline CVector<3> TransformPoint(const CVector<3> &v) const;
  inline CVector<3> TransformVector(const CVector<3> &v) const;

  inline void OrthonormalizeRotation();
  inline void OrthogonalizeRotation();
};

CVector<3> CXForm::GetTranslation() const
{
  CVector<3> vTrans = { (*m_pMatrix)(3, 0), (*m_pMatrix)(3, 1), (*m_pMatrix)(3, 2) };
  return vTrans;
}

CVector<3> CXForm::GetScale() const
{
  CVector<3> vScale;
  vScale.x() = m_pMatrix->GetRow(0).Length();
  vScale.y() = m_pMatrix->GetRow(1).Length();
  vScale.z() = m_pMatrix->GetRow(2).Length();
  return vScale;
}

CMatrix<3, 3> CXForm::GetRotation() const
{
  CMatrix<3, 3> mRot;
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      mRot(r, c) = (*m_pMatrix)(r, c);
  mRot.Normalize();
  return mRot;
}

void CXForm::SetTranslation(const CVector<3> &vTrans)
{
  (*m_pMatrix)(3, 0) = vTrans.x();
  (*m_pMatrix)(3, 1) = vTrans.y();
  (*m_pMatrix)(3, 2) = vTrans.z();
}

void CXForm::SetScale(const CVector<3> &vScale)
{
  m_pMatrix->GetRow(0).SetLength(vScale.x());
  m_pMatrix->GetRow(1).SetLength(vScale.y());
  m_pMatrix->GetRow(2).SetLength(vScale.z());
}

void CXForm::SetRotation(const CMatrix<3, 3> &mRotation)
{
  CVector<3> vScale = GetScale();
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      (*m_pMatrix)(r, c) = mRotation(r, c);
  SetScale(vScale);
}

CVector<3> CXForm::TransformPoint(const CVector<3> &v) const
{
  CVector<4> vRes;
  vRes = CVector<4>::Get(v.x(), v.y(), v.z(), 1) * (*m_pMatrix);
  return CVector<3>::Get(vRes.x(), vRes.y(), vRes.z());
}

CVector<3> CXForm::TransformVector(const CVector<3> &v) const
{
  CVector<4> vRes;
  vRes = CVector<4>::Get(v.x(), v.y(), v.z(), 0) * (*m_pMatrix);
  return CVector<3>::Get(vRes.x(), vRes.y(), vRes.z());
}

void CXForm::OrthonormalizeRotation()
{
  SetRotation(GetRotation().Orthonormalize());
}

void CXForm::OrthogonalizeRotation()
{
  SetRotation(GetRotation().Orthogonalize());
}

#endif