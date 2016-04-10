#include "stdafx.h"
#include "Shape.h"

CRTTIRegisterer<CShape3D> g_RegShape3D;
CRTTIRegisterer<CPoint3D> g_RegPoint3D;
CRTTIRegisterer<CLine3D> g_RegLine3D;
CRTTIRegisterer<CSegment3D> g_RegSegment3D;
CRTTIRegisterer<CRay3D> g_RegRay3D;
CRTTIRegisterer<CPlane> g_RegPlane;
CRTTIRegisterer<CHalfSpace> g_RegHalfSpace;
CRTTIRegisterer<CSphere> g_RegSphere;
CRTTIRegisterer<CAABB> g_RegAABB;
CRTTIRegisterer<COBB> g_RegOBB;

// CShape3D -------------------------------------------------------------------

const CShape3D::Num CShape3D::INVALID = Util::F_QNAN;

CShape3D::Num CShape3D::GetDistance(const CShape3D *pShape) const
{
  Num nDist = INVALID;
  if (!Dist(pShape, nDist))
    if (!pShape->Dist(this, nDist))
      ASSERT(!"Comparison operation for this combination of shapes not defined!");
  return nDist;
}

CShape3D *CShape3D::Clone() const
{
  CShape3D *pShape = (CShape3D *) GetRTTI()->CreateInstance();
  pShape->CopyFrom(this);
  return pShape;
}

// CPoint3D -------------------------------------------------------------------

void CPoint3D::Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount)
{
  ASSERT(pPoints && uiCount == 1);
  m_vPoint = Util::IndexStride(pPoints, iStride, 0);
}

void CPoint3D::CopyFrom(CShape3D const *pSrc)
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pSrc);
  m_vPoint = pPoint->m_vPoint;
}

bool CPoint3D::IsEmpty() const
{
  return IsInvalid(m_vPoint.x());
}

void CPoint3D::SetEmpty()
{
  m_vPoint.x() = INVALID;
}

CShape3D::Num CPoint3D::GetVolume() const
{
  return 0;
}

bool CPoint3D::Dist(const CShape3D *pShape, Num &nDist) const
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint) {
    nDist = Dist2Point(m_vPoint, pPoint->m_vPoint);
    return true;
  }
  return false;
}

CShape3D *CPoint3D::GetTransformed(CXForm const &kXForm, CShape3D *pDstShape) const
{
  CPoint3D *pPoint;
  if (pDstShape) {
    pPoint = Cast<CPoint3D>(pDstShape);
    ASSERT(pPoint);
    if (!pPoint)
      return 0;
  } else
    pPoint = NEW(CPoint3D, ());
  Transform(m_vPoint, kXForm, pPoint->m_vPoint);
  return pPoint;
}

void CPoint3D::Transform(CVector<3, Num> const &vPoint, CXForm const &kXForm, CVector<3, Num> &vXPoint)
{
  vXPoint = kXForm.TransformPoint(vPoint);
}

CShape3D::Num CPoint3D::Dist2Point(const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1)
{
  CVector<3, Num> vDiff = vPoint1 - vPoint0;
  return vDiff.Length();
}

// CLine3D --------------------------------------------------------------------

void CLine3D::Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount)
{
  ASSERT(pPoints && uiCount == 2);
  m_vPoints[0] = Util::IndexStride(pPoints, iStride, 0);
  m_vPoints[1] = Util::IndexStride(pPoints, iStride, 1);
}

void CLine3D::CopyFrom(CShape3D const *pSrc)
{
  const CLine3D *pLine = Cast<CLine3D>(pSrc);
  m_vPoints[0] = pLine->m_vPoints[0];
  m_vPoints[1] = pLine->m_vPoints[1];
}

bool CLine3D::IsEmpty() const
{
  return IsInvalid(m_vPoints[0].x());
}

void CLine3D::SetEmpty()
{
  m_vPoints[0].x() = INVALID;
}

CShape3D::Num CLine3D::GetVolume() const
{
  return 0;
}

bool CLine3D::Dist(const CShape3D *pShape, Num &nDist) const
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint) {
    nDist = Dist2Point(m_vPoints[0], m_vPoints[1], GetMinLimit(), GetMaxLimit(), pPoint->m_vPoint);
    return true;
  }
  const CLine3D *pLine = Cast<CLine3D>(pShape);
  if (pLine) {
    nDist = Dist2Line(m_vPoints[0], m_vPoints[1], GetMinLimit(), GetMaxLimit(),
                      pLine->m_vPoints[0], pLine->m_vPoints[1], pLine->GetMinLimit(), pLine->GetMaxLimit());
    return true;
  }
  return false;
}

CShape3D *CLine3D::GetTransformed(CXForm const &kXForm, CShape3D *pDstShape) const
{
  CLine3D *pLine;
  if(pDstShape) {
    pLine = Cast<CLine3D>(pDstShape);
    ASSERT(pLine && pLine->GetMinLimit() == GetMinLimit() && pLine->GetMaxLimit() == GetMaxLimit());
    if (!pLine || pLine->GetMinLimit() != GetMinLimit() || pLine->GetMaxLimit() != GetMaxLimit())
      return 0;
  } else
    pLine = (CLine3D *) GetRTTI()->CreateInstance();
  Transform(m_vPoints[0], m_vPoints[1], kXForm, pLine->m_vPoints[0], pLine->m_vPoints[1]);
  return pLine;
}

void CLine3D::Transform(CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1, CXForm const &kXForm,
                        CVector<3, Num> &vXPoint0, CVector<3, Num> &vXPoint1)
{
  vXPoint0 = kXForm.TransformPoint(vPoint0);
  vXPoint1 = kXForm.TransformPoint(vPoint1);
}

CShape3D::Num CLine3D::Dist2Line(const CVector<3, Num> &vLine0Point0, const CVector<3, Num> &vLine0Point1, Num nLine0Min, Num nLine0Max,
                                 const CVector<3, Num> &vLine1Point0, const CVector<3, Num> &vLine1Point1, Num nLine1Min, Num nLine1Max)
{
  CVector<3, Num> vDelta = vLine0Point1 - vLine0Point0;
  Num nDeltaDotDelta = vDelta % vDelta;
  CVector<3, Num> vDelta1 = vLine1Point1 - vLine1Point0;
  Num nDelta1DotDelta1 = vDelta1 % vDelta1;

  if (IsEqual(nDeltaDotDelta, 0)) {
    if (IsEqual(nDelta1DotDelta1, 0)) // Both lines are actually points
      return (vLine0Point0 - vLine1Point0).Length();
    else // Other line's a line and this one is a point
      return Dist2Point(vLine1Point0, vLine1Point1, nLine1Min, nLine1Max, vLine0Point0);
  }

  Num a, b, c, d, e, t, u;
  c = vDelta % vDelta1;
  if (IsEqual(nDelta1DotDelta1, 0) || IsEqual(abs(c), nDeltaDotDelta * nDelta1DotDelta1))
    return Dist2Point(vLine0Point0, vLine0Point1, nLine0Min, nLine0Max, vLine1Point0);
  a = (vLine0Point0 - vLine1Point0) % vDelta;
  e = (vLine0Point0 - vLine1Point0) % vDelta1;
  b = nDeltaDotDelta;
  d = nDelta1DotDelta1;

  t = (e + a * d / c) / (b * d / c - c);
  u = (a + t * b) / c;

  ASSERT(IsEqual((vLine0Point0 + t * vDelta - vLine1Point0 + u * vDelta1) % vDelta, 0) &&
         IsEqual((vLine0Point0 + t * vDelta - vLine1Point0 + u * vDelta1) % vDelta1, 0));

  t = Util::Bound(t, nLine0Min, nLine0Max);
  u = Util::Bound(u, nLine1Min, nLine1Max);

  CVector<3, Num> vClosest, vClosest1;
  vClosest = vLine0Point0 + t * vDelta;
  vClosest1 = vLine1Point0 + u * vDelta1;

  return (vClosest - vClosest1).Length();
/*
  The line that connects the closest points on the two shape lines is perpendicular to each of the shape lines

  (p0 + t * d - p10 - u * d1) % d = 0
  (p0 + t * d - p10 - u * d1) % d1 = 0

  (p0 - p10) % d + t * (d % d) - u * (d1 % d) = 0
  (p0 - p10) % d1 + t * (d % d1) - u * (d1 % d1) = 0

  a + t * b - u * c = 0
  e + t * c - u * d = 0

  u = (a + t * b) / c

  a + t * c - (a + t * b) * d / c = 0
  e + a * d / c + t * (c - b * d / c) = 0
  t = (e + a * d / c) / (b * d / c - c)
*/
}

CShape3D::Num CLine3D::Dist2Point(const CVector<3, Num> &vLinePoint0, const CVector<3, Num> &vLinePoint1, Num nLineMin, Num nLineMax,
                                  const CVector<3, Num> &vPoint)
{
  CVector<3, Num> vDelta = vLinePoint1 - vLinePoint0;
  Num nDeltaDotDelta = vDelta % vDelta;
  if (IsEqual(nDeltaDotDelta, 0))
    return (vLinePoint0 - vPoint).Length();
  Num t = (vPoint - vLinePoint0) % vDelta;
  t /= nDeltaDotDelta;
  t = Util::Bound(t, nLineMin, nLineMax);
  CVector<3, Num> vClosest = vLinePoint0 + vDelta * t;
  ASSERT(IsEqual((vClosest - vPoint) % vDelta, 0));
  return (vClosest - vPoint).Length();
/*
  The closest point on the shape line lies on a line that's perpendicular to the direction of the shape line
  (p0 + t * d - p) % d = 0
  (p0 - p) % d + t * (d % d) = 0
  ((p - p0) % d) / (d % d) = t
*/
}

// CPlane ---------------------------------------------------------------------

void CPlane::Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount)
{
  CVector<3, Num> vNormal;
  ASSERT(pPoints && uiCount == 3);
  vNormal = (Util::IndexStride(pPoints, iStride, 1) - Util::IndexStride(pPoints, iStride, 0)) ^ (Util::IndexStride(pPoints, iStride, 2) - Util::IndexStride(pPoints, iStride, 0));
  vNormal.Normalize();
  m_vPlane.Set(vNormal);
  m_vPlane.w() = - (vNormal % Util::IndexStride(pPoints, iStride, 0));
}

void CPlane::CopyFrom(CShape3D const *pSrc)
{
  const CPlane *pPlane = Cast<CPlane>(pSrc);
  m_vPlane = pPlane->m_vPlane;
}

bool CPlane::IsEmpty() const
{
  return IsInvalid(m_vPlane.x());
}

void CPlane::SetEmpty()
{
  m_vPlane.x() = INVALID;
}

CShape3D::Num CPlane::GetVolume() const
{
  ASSERT(!IsEmpty());
  if (IsHalfSpace())
    return Util::F_INFINITY;
  return 0;
}

bool CPlane::Dist(const CShape3D *pShape, Num &nDist) const
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint) {
    nDist = Dist2Point(m_vPlane, IsHalfSpace(), pPoint->m_vPoint);
    return true;
  }
  const CLine3D *pLine = Cast<CLine3D>(pShape);
  if (pLine) {
    nDist = Dist2Line(m_vPlane, IsHalfSpace(), pLine->m_vPoints[0], pLine->m_vPoints[1], pLine->GetMinLimit(), pLine->GetMaxLimit());
    return true;
  }
  const CPlane *pPlane = Cast<CPlane>(pShape);
  if (pPlane) {
    nDist = Dist2Plane(m_vPlane, IsHalfSpace(), pPlane->m_vPlane, pPlane->IsHalfSpace());
    return true;
  }
  return false;
}

CShape3D *CPlane::GetTransformed(CXForm const &kXForm, CShape3D *pDstShape) const
{
  CPlane *pPlane;
  if (pDstShape) {
    pPlane = Cast<CPlane>(pDstShape);
    ASSERT(pPlane && pPlane->IsHalfSpace() == IsHalfSpace());
    if (!pPlane || pPlane->IsHalfSpace() != IsHalfSpace())
      return 0;
  } else
    pPlane = (CPlane *) GetRTTI()->CreateInstance();
  Transform(m_vPlane, kXForm, pPlane->m_vPlane);
  return pPlane;
}

void CPlane::Transform(CVector<4, Num> const &vPlane, CXForm const &kXForm, CVector<4, Num> &vXPlane)
{
  // A plane is transformed by multiplying to the inverse transpose of the original matrix
  // To avoid transposing, we multiply as a vector - column instead of as a row
  CMatrix<4, 4> mInv;
  kXForm.m_pMatrix->GetInverse(mInv);
  vXPlane = mInv * vPlane;
}

CVector<3, CShape3D::Num> CPlane::GetPoint(const CVector<4, Num> &vPlane)
{
  Num nFactor;
  CVector<3, Num> vPoint, vNormal;
  vNormal.Set(vPlane);
  nFactor = -vPlane.w() / vNormal.Length();
  vPoint.Set(vPlane);
  vPoint *= nFactor;
  ASSERT(IsEqual(vPoint.x() * vPlane.x() + vPoint.y() * vPlane.y() + vPoint.z() * vPlane.z() + vPlane.w(), 0));
  return vPoint;
}

CShape3D::Num CPlane::GetIntersectionFactor(CVector<4, Num> const &vPlane, CVector<3, Num> const &vOrigin, CVector<3, Num> const &vDirection)
{
  CVector<3, Num> vNorm;
  Num nDist, nNormDotLine, t;
  vNorm.Set(vPlane);
  nNormDotLine = vNorm % vDirection;
  if (IsEqual(nNormDotLine, 0)) { // Line is parallel to the plane
    nDist = vOrigin.x() * vPlane.x() + vOrigin.y() * vPlane.y() + vOrigin.z() * vPlane.z() + vPlane.w();
    if (IsEqual(nDist, 0))
      return 0;
    return Util::F_QNAN;
  }
  t = -(vOrigin % vNorm + vPlane.w()) / nNormDotLine;
  return t;
}

bool CPlane::Intersect2Planes(CVector<4, Num> const &vPlane0, CVector<4, Num> const &vPlane1,
                              CVector<4, Num> const &vPlane2, CVector<3, Num> &vPoint)
{
  CMatrix<3, 4, Num> mLinearEq;
  mLinearEq.SetRow(0, vPlane0);
  mLinearEq.SetRow(1, vPlane1);
  mLinearEq.SetRow(2, vPlane2);
  if (!mLinearEq.MakeTriangular<3>(true))
    return false;
  mLinearEq.MakeZeroAboveDiagonal<3>();
  vPoint.x() = -mLinearEq(0, 3);
  vPoint.y() = -mLinearEq(1, 3);
  vPoint.z() = -mLinearEq(2, 3);
#ifdef _DEBUG
  Num n = CVector<4, Num>::Get(vPoint.x(), vPoint.y(), vPoint.z(), 1) % vPlane0;
  Num n1 = CVector<4, Num>::Get(vPoint.x(), vPoint.y(), vPoint.z(), 1) % vPlane1;
  Num n2 = CVector<4, Num>::Get(vPoint.x(), vPoint.y(), vPoint.z(), 1) % vPlane2;
#endif
/*
  ASSERT(ID(CONCAT(IsEqual(CVector<4, Num>::Get(vPoint.x(), vPoint.y(), vPoint.z(), 1) % vPlane0, 0, (Num) 0.005))));
  ASSERT(ID(CONCAT(IsEqual(CVector<4, Num>::Get(vPoint.x(), vPoint.y(), vPoint.z(), 1) % vPlane1, 0, (Num) 0.005))));
  ASSERT(ID(CONCAT(IsEqual(CVector<4, Num>::Get(vPoint.x(), vPoint.y(), vPoint.z(), 1) % vPlane2, 0, (Num) 0.005))));
*/
  return true;
}

bool CPlane::IntersectPlane(CVector<4, Num> const &vPlane0, CVector<4, Num> const &vPlane1,
                            CVector<3, Num> &vLine0, CVector<3, Num> &vLine1)
{
  CVector<3, Num> vBasis[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
  int iMin, i;
  float fCur, fMin = Util::F_INFINITY;
  CVector<3, Num> vNorm[2];
  CVector<4, Num> vPlane;

  vNorm[0].Set(vPlane0).Normalize();
  vNorm[1].Set(vPlane1).Normalize();

  // Find a direction that is least colinear with the given plane normals
  for (i = 0; i < 3; i++) {
    fCur = Util::Max(abs(vNorm[0] % vBasis[i]), abs(vNorm[1] % vBasis[i]));
    if (fCur < fMin) {
      iMin = i;
      fMin = fCur;
    }
  }

  // Make 2 different planes with that plane normal and find the intersection points of the original planes with each of the new ones
  // These intersection points define the plane intersection line
  vPlane.Set(vBasis[iMin]);
  vPlane.w() = 0;
  if (!Intersect2Planes(vPlane0, vPlane1, vPlane, vLine0))
    return false;
  vPlane.w() = 1;
  if (!Intersect2Planes(vPlane0, vPlane1, vPlane, vLine1))
    return false;
  return true;
}

bool CPlane::IntersectLine(CVector<4, Num> const &vPlane, CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1,
                           Num nLineMin, Num nLineMax, CVector<3, Num> &vPoint)
{
  CVector<3, Num> vLine;
  Num t;
  vLine = vPoint1 - vPoint0;
  t = GetIntersectionFactor(vPlane, vPoint0, vLine);
  if (t == Util::F_QNAN)
    return false;
  if (t < nLineMin)
    t = nLineMin;
  else
    if (t > nLineMax)
      t = nLineMax;
    else // Intersection point is a valid part of the line, distance is 0
      return false;
  vPoint = vPoint0 + vLine * t;
  return true;
}

CShape3D::Num CPlane::Dist2Plane(const CVector<4, Num> &vPlane0, bool bHalfSpace0, const CVector<4, Num> &vPlane1, bool bHalfSpace1)
{
  Num nFactor, nDist;
  CVector<3, Num> vNorm, vPlaneNorm;
  CVector<3, Num> vPoint;
  vNorm.Set(vPlane0);
  vPlaneNorm.Set(vPlane1);
  if (!vNorm.IsParallel(vPlaneNorm, &nFactor)) // Planes aren't parallel, so they intersect
    return 0;
  if (bHalfSpace0 && bHalfSpace1 && nFactor >= 0) // Planes aren't facing in opposite directions, so one half-space is inside the other
    return 0;
  if (!bHalfSpace0) {
    vPoint = CPlane::GetPoint(vPlane0);
    nDist = Dist2Point(vPlane1, bHalfSpace1, vPoint);
  } else {
    vPoint = CPlane::GetPoint(vPlane1);
    nDist = Dist2Point(vPlane0, bHalfSpace0, vPoint);
  }
  return nDist;
}

CShape3D::Num CPlane::Dist2Line(const CVector<4, Num> &vPlane, bool bHalfSpace,
                                const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax)
{
  CVector<3, Num> vLine, vPoint;
  Num nDist, t;
  vLine = vPoint1 - vPoint0;
  t = GetIntersectionFactor(vPlane, vPoint0, vLine);
  if (t == Util::F_QNAN) { // Line is parallel to the plane
    nDist = Dist2Point(vPlane, bHalfSpace, vPoint0);
    return nDist;
  }
  if (t < nLineMin)
    t = nLineMin;
  else
    if (t > nLineMax)
      t = nLineMax;
    else // Intersection point is a valid part of the line, distance is 0
      return 0;
  vPoint = vPoint0 + vLine * t;
  nDist = Dist2Point(vPlane, bHalfSpace, vPoint);
  return nDist;
}

CShape3D::Num CPlane::Dist2Point(const CVector<4, Num> &vPlane, bool bHalfSpace, const CVector<3, Num> &vPoint)
{
  Num nDist, nNormLen;
  nDist = vPoint.x() * vPlane.x() + vPoint.y() * vPlane.y() + vPoint.z() * vPlane.z() + vPlane.w();
  if (bHalfSpace && nDist < 0) // Point is on the negative side, so it's in the half-space occupied by this plane shape
    return 0;
  nNormLen = CVector<3, Num>::Get(vPlane).Length();
  nDist /= nNormLen;
  return nDist;
}

// CSphere --------------------------------------------------------------------

void CSphere::Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount)
{
  CVector<3, Num> vMin, vMax;
  int i;
  Num nDist;

  CAABB::CalcFromPoints(pPoints, iStride, uiCount, vMin, vMax);
  m_vCenter = (vMin + vMax) * 0.5;
  m_nRadius = 0;
  for (i = 0; i < (int) uiCount; i++) {
    nDist = (Util::IndexStride(pPoints, iStride, i) - m_vCenter).LengthSqr();
    if (nDist > m_nRadius)
      m_nRadius = nDist;
  }
  m_nRadius = sqrt(m_nRadius);
}

void CSphere::CopyFrom(CShape3D const *pSrc)
{
  const CSphere *pSphere = Cast<CSphere>(pSrc);
  m_vCenter = pSphere->m_vCenter;
  m_nRadius = pSphere->m_nRadius;
}

bool CSphere::IsEmpty() const
{
  return IsInvalid(m_nRadius);
}

void CSphere::SetEmpty()
{
  m_nRadius = INVALID;
}

void CSphere::IncludePoint(CVector<3> const &v)
{
  if (IsEmpty()) {
    m_vCenter = v;
    m_nRadius = 0;
    return;
  }
  Num nDist = (v - m_vCenter).Length();
  if (nDist <= m_nRadius)
    return;
  Num nW = (nDist - m_nRadius) / (2 * nDist);
  m_vCenter = Util::Lerp(m_vCenter, v, nW);
  m_nRadius = (nDist + m_nRadius) / 2;
  ASSERT(IsEqual((v - m_vCenter).Length(), m_nRadius));
}

CShape3D::Num CSphere::GetVolume() const
{
  Num nVolume;
  nVolume = m_nRadius * m_nRadius * m_nRadius * 4 * PI / 3;
  return nVolume;
}

bool CSphere::Dist(const CShape3D *pShape, Num &nDist) const
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint) {
    nDist = Dist2Point(m_vCenter, m_nRadius, pPoint->m_vPoint);
    return true;
  }
  const CLine3D *pLine = Cast<CLine3D>(pShape);
  if (pLine) {
    nDist = Dist2Line(m_vCenter, m_nRadius, pLine->m_vPoints[0], pLine->m_vPoints[1], pLine->GetMinLimit(), pLine->GetMaxLimit());
    return true;
  }
  const CPlane *pPlane = Cast<CPlane>(pShape);
  if (pPlane) {
    nDist = Dist2Plane(m_vCenter, m_nRadius, pPlane->m_vPlane, pPlane->IsHalfSpace());
    return true;
  }
  const CSphere *pSphere = Cast<CSphere>(pShape);
  if (pSphere) {
    nDist = Dist2Sphere(m_vCenter, m_nRadius, pSphere->m_vCenter, pSphere->m_nRadius);
    return true;
  }
  return false;
}

CShape3D *CSphere::GetTransformed(CXForm const &kXForm, CShape3D *pDstShape) const
{
  CSphere *pSphere;
  if (pDstShape) {
    pSphere = Cast<CSphere>(pDstShape);
    ASSERT(pSphere);
    if (!pSphere)
      return 0;
  } else
    pSphere = NEW(CSphere, ());
  Transform(m_vCenter, m_nRadius, kXForm, pSphere->m_vCenter, pSphere->m_nRadius);
  return pSphere;
}

void CSphere::Transform(CVector<3, Num> const &vCenter, Num nRadius, CXForm const &kXForm,
                        CVector<3, Num> &vXCenter, Num &nXRadius)
{
  CVector<3, Num> vScale;
  vScale = kXForm.GetScale();
  ASSERT(IsEqual(vScale.x(), vScale.y()) && IsEqual(vScale.y(), vScale.z()));
  vXCenter = kXForm.TransformPoint(vCenter);
  nXRadius = nRadius * vScale.x();
}

bool CSphere::IntersectLine(CVector<3, Num> const &vCenter, Num nRadius,
                            CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1,
                            Num nLineMin, Num nLineMax, Num &nMinFactor, Num &nMaxFactor)
{
  CVector<3, Num> vLine, vDelta;
  Num nDist;
  int iRoots;

  vLine = vPoint1 - vPoint0;
  if (IsEqual(vLine.LengthSqr(), 0)) { // Line is actually a point
    nDist = Dist2Point(vCenter, nRadius, vPoint0);
    if (nDist > 0)
      return false;
    nMinFactor = nMaxFactor = 0;
    return true;
  }

  vDelta = vPoint0 - vCenter;
  iRoots = Util::SolveQuadratic(vLine % vLine, 2 * (vLine % vDelta), vDelta % vDelta - nRadius * nRadius, nMinFactor, nMaxFactor);

  if (iRoots <= 0)
    return false;

  ASSERT(nMinFactor <= nMaxFactor);
  if (nMinFactor > nLineMax)
    return false;
  if (nMaxFactor < nLineMin)
    return false;

  nMinFactor = Util::Bound(nMinFactor, nLineMin, nLineMax);
  nMaxFactor = Util::Bound(nMaxFactor, nLineMin, nLineMax);
  return true;
}

CShape3D::Num CSphere::Dist2Sphere(const CVector<3, Num> &vCenter0, Num nRadius0, const CVector<3, Num> &vCenter1, Num nRadius1)
{
  Num nDist;
  nDist = (vCenter1 - vCenter0).Length() - nRadius0 - nRadius1;
  if (nDist < 0)
    nDist = 0;
  return nDist;
}

CShape3D::Num CSphere::Dist2Plane(const CVector<3, Num> &vCenter, Num nRadius, const CVector<4, Num> &vPlane, bool bHalfSpace)
{
  Num nDist;
  nDist = CPlane::Dist2Point(vPlane, bHalfSpace, vCenter);
  // No check is made for half-space case since the distance will already be clamped to 0 in this case
  ASSERT(nDist >= 0 || !bHalfSpace);
  if (nDist < 0)
    nDist = Util::Min<Num>(nDist + nRadius, 0);
  else
    nDist = Util::Max<Num>(nDist - nRadius, 0);
  return nDist;
}

CShape3D::Num CSphere::Dist2Line(const CVector<3, Num> &vCenter, Num nRadius,
                                 const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax)
{
  Num nDist;
  nDist = CLine3D::Dist2Point(vPoint0, vPoint1, nLineMin, nLineMax, vCenter) - nRadius;
  if (nDist < 0)
    nDist = 0;
  return nDist;
}

CShape3D::Num CSphere::Dist2Point(const CVector<3, Num> &vCenter, Num nRadius, const CVector<3, Num> &vPoint)
{
  Num nDist;
  nDist = (vPoint - vCenter).Length() - nRadius;
  if (nDist < 0)
    nDist = 0;
  return nDist;
}

// CAABB ----------------------------------------------------------------------

void CAABB::Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount)
{
  CalcFromPoints(pPoints, iStride, uiCount, m_vMin, m_vMax);
}

void CAABB::IncludeAABB(CAABB const &kAABB)
{
  int i;
  if (kAABB.IsEmpty())
    return;
  if (IsEmpty()) {
    *this = kAABB;
    return;
  }
  for (i = 0; i < 3; i++) {
    if (m_vMin[i] > kAABB.m_vMin[i])
      m_vMin[i] = kAABB.m_vMin[i];
    if (m_vMax[i] < kAABB.m_vMax[i])
      m_vMax[i] = kAABB.m_vMax[i];
  }
}

void CAABB::CopyFrom(CShape3D const *pSrc)
{
  const CAABB *pAABB = Cast<CAABB>(pSrc);
  m_vMin = pAABB->m_vMin;
  m_vMax = pAABB->m_vMax;
}

bool CAABB::IsEmpty() const
{
  return IsInvalid(m_vMin.x());
}

void CAABB::SetEmpty()
{
  m_vMin.x() = INVALID;
}

void CAABB::IncludePoint(CVector<3> const &v)
{
  if (IsEmpty()) {
    m_vMin = v;
    m_vMax = v;
    return;
  }

  int i;
  for (i = 0; i < 3; i++)
    if (m_vMin[i] > v[i])
      m_vMin[i] = v[i];
    else
      if (m_vMax[i] < v[i])
        m_vMax[i] = v[i];
}

CShape3D::Num CAABB::GetVolume() const
{
  CVector<3, Num> vDelta = m_vMax - m_vMin;
  Num nDist = vDelta.x() * vDelta.y() * vDelta.z();
  return nDist;
}

bool CAABB::Dist(const CShape3D *pShape, Num &nDist) const
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint) {
    nDist = Dist2Point(m_vMin, m_vMax, pPoint->m_vPoint);
    return true;
  }
  const CLine3D *pLine = Cast<CLine3D>(pShape);
  if (pLine) {
    nDist = Dist2Line(m_vMin, m_vMax, pLine->m_vPoints[0], pLine->m_vPoints[1], pLine->GetMinLimit(), pLine->GetMaxLimit());
    return true;
  }
  const CPlane *pPlane = Cast<CPlane>(pShape);
  if (pPlane) {
    nDist = Dist2Plane(m_vMin, m_vMax, pPlane->m_vPlane, pPlane->IsHalfSpace());
    return true;
  }
  const CSphere *pSphere = Cast<CSphere>(pShape);
  if (pSphere) {
    nDist = Dist2Sphere(m_vMin, m_vMax, pSphere->m_vCenter, pSphere->m_nRadius);
    return true;
  }
  const CAABB *pAABB = Cast<CAABB>(pShape);
  if (pAABB) {
    nDist = Dist2AABB(m_vMin, m_vMax, pAABB->m_vMin, pAABB->m_vMax);
    return true;
  }
  return false;

}

CShape3D *CAABB::GetTransformed(CXForm const &kXForm, CShape3D *pDstShape) const
{
  COBB *pOBB;
  CAABB *pAABB;
  if (pDstShape) {
    pOBB = Cast<COBB>(pDstShape);
    if (!pOBB) {
      pAABB = Cast<CAABB>(pDstShape);
      ASSERT(pAABB);
      if (!pAABB)
        return 0;
      Transform(m_vMin, m_vMax, kXForm, pAABB->m_vMin, pAABB->m_vMax);
      return pAABB;
    }
  } else
    pOBB = NEW(COBB, ());
  Transform(m_vMin, m_vMax, kXForm, pOBB->m_vCenter, pOBB->m_vExtent);
  return pOBB;
}

void CAABB::Transform(CVector<3, Num> const &vMin, CVector<3, Num> const &vMax, CXForm const &kXForm,
                      CVector<3, Num> &vXCenter, CVector<3, Num> (&vXExtent)[3])
{
  int i;
  vXCenter = (vMin + vMax) * 0.5;
  vXCenter = kXForm.TransformPoint(vXCenter);
  for (i = 0; i < 3; i++) {
    vXExtent[i].SetZero();
    vXExtent[i][i] = (vMax[i] - vMin[i]) / 2;
    vXExtent[i] = kXForm.TransformVector(vXExtent[i]);
  }
}

void CAABB::Transform(CVector<3, Num> const &vMin, CVector<3, Num> const &vMax, CXForm const &kXForm,
                      CVector<3, Num> &vXMin, CVector<3, Num> &vXMax)
{
  CVector<3, Num> vXCenter, vXExtent[3];
  int i;

  Transform(vMin, vMax, kXForm, vXCenter, vXExtent);
  for (i = 0; i < 3; i++) {
    vXMin[i] = vXCenter[i] - abs(vXExtent[0][i]) - abs(vXExtent[1][i]) - abs(vXExtent[2][i]);
    vXMax[i] = vXCenter[i] + abs(vXExtent[0][i]) + abs(vXExtent[1][i]) + abs(vXExtent[2][i]);
  }
}


CShape3D::Num CAABB::Dist2AABB(const CVector<3, Num> &vMin0, const CVector<3, Num> &vMax0,
                               const CVector<3, Num> &vMin1, const CVector<3, Num> &vMax1)
{
  CVector<3, Num> vClosest0, vClosest1;
  int i;
  Num nDist;
  for (i = 0; i < 3; i++) { // Find the closest per-coordinate values that belong to each of the AABBs
    if (vMin0[i] > vMax1[i]) {
      vClosest0[i] = vMin0[i];
      vClosest1[i] = vMax1[i];
    } else
      if (vMin1[i] > vMax0[i]) {
        vClosest0[i] = vMax0[i];
        vClosest1[i] = vMin1[i];
      } else
        vClosest0[i] = vClosest1[i] = Util::Max(vMin0[i], vMin1[i]);
  }
  nDist = (vClosest1 - vClosest0).Length();
  return nDist;
}

CShape3D::Num CAABB::Dist2Sphere(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                                 const CVector<3, Num> &vCenter, Num nRadius)
{
  Num nDist = Dist2Point(vMin, vMax, vCenter) - nRadius;
  if (nDist < 0)
    nDist = 0;
  return nDist;
}

CShape3D::Num CAABB::Dist2Plane(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax, const CVector<4, Num> &vPlane, bool bHalfSpace)
{
  CVector<3, Num> vClosest, vFurthest, vNorm;
  int i;
  Num nDist;
  vNorm.Set(vPlane);
  for (i = 0; i < 3; i++)
    if (vNorm[i] > 0) {
      vClosest[i] = vMin[i];
      vFurthest[i] = vMax[i];
    } else {
      vClosest[i] = vMax[i];
      vFurthest[i] = vMin[i];
    }
  nDist = CPlane::Dist2Point(vPlane, bHalfSpace, vClosest);
  if (nDist < 0) { // Can only happen if the plane is not a half-space
    ASSERT(!bHalfSpace);
    nDist = CPlane::Dist2Point(vPlane, bHalfSpace, vFurthest);
    if (nDist >= 0) // Points at the extremes of the AABB are on different sides of the plane
      return 0;
  }
  return nDist;
}

bool CAABB::TestLineCoordinate(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                               const CVector<3, Num> &vOrigin, const CVector<3, Num> &vDirection,
                               Num nLineMin, Num nLineMax, Num nCoordinate, int iDim, Num &nFactor)
{
  CVector<3, Num> vPoint;
  int j;

  nFactor = (nCoordinate - vOrigin[iDim]) / vDirection[iDim];
  if (nFactor < nLineMin || nFactor > nLineMax)
    return false; // Intersection point is outside the valid part of the line
  vPoint = vOrigin + vDirection * nFactor;
  ASSERT(IsEqual(vPoint[iDim], nCoordinate));
  for (j = 0; j < 3; j++) {
    if (iDim == j)
      continue;
    if (vPoint[j] < vMin[j] || vPoint[j] > vMax[j])
      return false;  // Intersection point is outside the rectangle
  }
  return true;  // Intersection point is inside rectangle
}

CShape3D::Num CAABB::Edge2LineDist(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax, int iDim, int iNumEdge,
                                   const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax)
{
  CVector<3, Num> vEdge0, vEdge1;
  int iCurDim;
  Num nDist;
  vEdge0[iDim] = vMin[iDim];
  vEdge1[iDim] = vMax[iDim];
  iCurDim = (iDim + 1) % 3;
  vEdge0[iCurDim] = (iNumEdge & 1) ? vMax[iCurDim] : vMin[iCurDim];
  vEdge1[iCurDim] = vEdge0[iCurDim];
  iCurDim = (iDim + 2) % 3;
  vEdge0[iCurDim] = (iNumEdge & 2) ? vMax[iCurDim] : vMin[iCurDim];
  vEdge1[iCurDim] = vEdge0[iCurDim];
  nDist = CLine3D::Dist2Line(vEdge0, vEdge1, 0, 1, vPoint0, vPoint1, nLineMin, nLineMax);
  return nDist;
}

bool CAABB::IntersectLine(CVector<3, Num> const &vMin, CVector<3, Num> const &vMax,
                          CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1,
                          Num nLineMin, Num nLineMax, Num &nMinFactor, Num &nMaxFactor)
{
  CVector<3, Num> vLine;
  int i;
  Num nFactor;

  vLine = vPoint1 - vPoint0;
  if (IsEqual(vLine.LengthSqr(), 0)) {
    if (vPoint0 >= vMin && vPoint0 <= vMax) {
      nMinFactor = nMaxFactor = 0;
      return true;
    }
    return false;
  }
  // Check for intersections with the sides
  nMinFactor = Util::F_INFINITY;
  nMaxFactor = Util::F_NEG_INFINITY;
  for (i = 0; i < 3; i++) {
    if (IsEqual(vLine[i], 0)) { // Line is parallel to the AABB sides in this dimension
      if (vPoint0[i] >= vMin[i] && vPoint0[i] <= vMax[i]) // Coordinate is within bounds, continue checking the other dimensions
        continue;
      break; // Line can't intersect the AABB
    }
    if (TestLineCoordinate(vMin, vMax, vPoint0, vLine, Util::F_NEG_INFINITY, Util::F_INFINITY, vMin[i], i, nFactor)) { // Line intersects the Min plane of the dimension inside the AABB side
      if (nFactor < nMinFactor)
        nMinFactor = nFactor;
      if (nFactor > nMaxFactor)
        nMaxFactor = nFactor;
    }
    if (TestLineCoordinate(vMin, vMax, vPoint0, vLine, Util::F_NEG_INFINITY, Util::F_INFINITY, vMax[i], i, nFactor)) { // Line intersects the Max plane of the dimension inside the AABB side
      if (nFactor < nMinFactor)
        nMinFactor = nFactor;
      if (nFactor > nMaxFactor)
        nMaxFactor = nFactor;
    }
  }
  if (nMinFactor == Util::F_INFINITY)
    return false;
  ASSERT(nMinFactor <= nMaxFactor);
  if (nMaxFactor < nLineMin || nMinFactor > nLineMax)
    return false;
  nMinFactor = Util::Bound(nMinFactor, nLineMin, nLineMax);
  nMaxFactor = Util::Bound(nMaxFactor, nLineMin, nLineMax);
  return true;
}


CShape3D::Num CAABB::Dist2Line(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                               const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax)
{
  CVector<3, Num> vLine;
  int i, j;
  Num nDist, nFactor;

  vLine = vPoint1 - vPoint0;
  if (IsEqual(vLine.LengthSqr(), 0))
    return Dist2Point(vMin, vMax, vPoint0);
  // Check for intersections with the sides
  for (i = 0; i < 3; i++) {
    if (IsEqual(vLine[i], 0)) { // Line is parallel to the AABB sides in this dimension
      if (vPoint0[i] >= vMin[i] && vPoint0[i] <= vMax[i]) // Coordinate is within bounds, continue checking the other dimensions
        continue;
      break; // Line can't intersect the AABB
    }
    if (TestLineCoordinate(vMin, vMax, vPoint0, vLine, nLineMin, nLineMax, vMin[i], i, nFactor)) // Line intersects the Min plane of the dimension inside the AABB side
      return 0;
    if (TestLineCoordinate(vMin, vMax, vPoint0, vLine, nLineMin, nLineMax, vMax[i], i, nFactor)) // Line intersects the Max plane of the dimension inside the AABB side
      return 0;
  }
  // No side intersections found, check AABB edges for the least distance to the line
  nDist = Util::F_INFINITY;
  for (i = 0; i < 3; i++)
    for (j = 0; j < 4; j++) {
      Num nEdgeDist = Edge2LineDist(vMin, vMax, i, j, vPoint0, vPoint1, nLineMin, nLineMax);
      if (nDist > nEdgeDist)
        nDist = nEdgeDist;
    }
  return nDist;
}

CShape3D::Num CAABB::Dist2Point(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax, const CVector<3, Num> &vPoint)
{
  CVector<3, Num> vClosest;
  int i;
  Num nDist;
  for (i = 0; i < 3; i++)
    vClosest[i] = Util::Bound(vPoint[i], vMin[i], vMax[i]);
  nDist = (vClosest - vPoint).Length();
  return nDist;
}

void CAABB::CalcFromPoints(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount,
                           CVector<3, Num> &vMin, CVector<3, Num> &vMax)
{
  int i, j;

  ASSERT(pPoints && uiCount);
  vMin = Util::IndexStride(pPoints, iStride, 0);
  vMax = vMin;
  for (i = 1; i < (int) uiCount; i++) {
    CVector<3, Num> const &vPt = Util::IndexStride(pPoints, iStride, i);
    for (j = 0; j < 3; j++) {
      if (vMin[j] > vPt[j])
        vMin[j] = vPt[j];
      else
        if (vMax[j] < vPt[j])
          vMax[j] = vPt[j];
    }
  }
}

// COBB -----------------------------------------------------------------------

void COBB::Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount)
{
  // The best fitting OBB is either supported by one side of the convex hull of the point set, or by three perpendicular edges of the convex hull
  // Since that's too much of a hassle, we just create the OBB as an AABB and be done with it
  CVector<3, Num> vMin, vMax;
  int i;

  CAABB::CalcFromPoints(pPoints, iStride, uiCount, vMin, vMax);
  m_vCenter = (vMin + vMax) * 0.5;
  for (i = 0; i < 3; i++) {
    m_vExtent[i].SetZero();
    m_vExtent[i][i] = (vMax[i] - vMin[i]) / 2;
  }
}

void COBB::CopyFrom(CShape3D const *pSrc)
{
  const COBB *pOBB = Cast<COBB>(pSrc);
  m_vCenter = pOBB->m_vCenter;
  m_vExtent[0] = pOBB->m_vExtent[0];
  m_vExtent[1] = pOBB->m_vExtent[1];
  m_vExtent[2] = pOBB->m_vExtent[2];
}

bool COBB::IsEmpty() const
{
  return IsInvalid(m_vCenter.x());
}

void COBB::SetEmpty()
{
  m_vCenter.x() = INVALID;
}

void COBB::IncludePoint(CVector<3> const &v)
{
  int i;
  if (IsEmpty()) {
    m_vCenter = v;
    for (i = 0; i < 3; i++)
      m_vExtent[i].SetZero();
    return;
  }
  CVector<3> vBase;
  Num nLen, nDist, nExtra;
  CVector<3> vDelta = v - m_vCenter;
  for (i = 0; i < 3; i++) {
    nLen = m_vExtent[i].Length();
    if (IsEqual(nLen, 0))
      vBase.SetUnit(i);
    else
      vBase = m_vExtent[i] * (1 / nLen);
    nDist = vBase % vDelta;
    if (abs(nDist) <= nLen)
      continue;
    nExtra = (abs(nDist) - nLen) / 2;
    m_vCenter += vBase * (Util::Sign(nDist) * nExtra);
    m_vExtent[i] += vBase * nExtra;
  }
}

CShape3D::Num COBB::GetVolume() const
{
  Num nVolume;
  nVolume = m_vExtent[0].LengthSqr() * m_vExtent[1].LengthSqr() * m_vExtent[2].LengthSqr() * 4 * 4 * 4;
  nVolume = sqrt(nVolume);
  return nVolume;
}

bool COBB::Dist(const CShape3D *pShape, Num &nDist) const
{
  const CPoint3D *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint) {
    nDist = Dist2Point(m_vCenter, m_vExtent, pPoint->m_vPoint);
    return true;
  }
  const CLine3D *pLine = Cast<CLine3D>(pShape);
  if (pLine) {
    nDist = Dist2Line(m_vCenter, m_vExtent, pLine->m_vPoints[0], pLine->m_vPoints[1], pLine->GetMinLimit(), pLine->GetMaxLimit());
    return true;
  }
  const CPlane *pPlane = Cast<CPlane>(pShape);
  if (pPlane) {
    nDist = Dist2Plane(m_vCenter, m_vExtent, pPlane->m_vPlane, pPlane->IsHalfSpace());
    return true;
  }
  const CSphere *pSphere = Cast<CSphere>(pShape);
  if (pSphere) {
    nDist = Dist2Sphere(m_vCenter, m_vExtent, pSphere->m_vCenter, pSphere->m_nRadius);
    return true;
  }
  const CAABB *pAABB = Cast<CAABB>(pShape);
  if (pAABB) {
    nDist = Dist2AABB(m_vCenter, m_vExtent, pAABB->m_vMin, pAABB->m_vMax);
    return true;
  }
  const COBB *pOBB = Cast<COBB>(pShape);
  if (pOBB) {
    nDist = Dist2OBB(m_vCenter, m_vExtent, pOBB->m_vCenter, pOBB->m_vExtent);
    return true;
  }
  return false;
}

CShape3D *COBB::GetTransformed(CXForm const &kXForm, CShape3D *pDstShape) const
{
  COBB *pOBB;
  if (pDstShape) {
    pOBB = Cast<COBB>(pDstShape);
    ASSERT(pOBB);
    if (!pOBB)
      return 0;
  } else
    pOBB = NEW(COBB, ());
  Transform(m_vCenter, m_vExtent, kXForm, pOBB->m_vCenter, pOBB->m_vExtent);
  return pOBB;
}

void COBB::Transform(CVector<3, Num> const &vCenter, CVector<3, Num> const (&vExtent)[3], CXForm const &kXForm,
                     CVector<3, Num> &vXCenter, CVector<3, Num> (&vXExtent)[3])
{
  int i;
  vXCenter = kXForm.TransformPoint(vCenter);
  for (i = 0; i < 3; i++)
    vXExtent[i] = kXForm.TransformVector(vExtent[i]);
}

void COBB::Untransform(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                       CVector<3, Num> &vMin, CVector<3, Num> &vMax, CXForm &kXForm)
{
  int i;
  CMatrix<3, 3, Num> mRot;
  for (i = 0; i < 3; i++) {
    vMax[i] = vExtent[i].Length();
    vMin[i] = -vMax[i];
    mRot.SetRow(i, vExtent[i] * (1 / vMax[i]));
  }
  kXForm.SetRotation(mRot);
  kXForm.SetTranslation(vCenter);
  kXForm.SetScale(CVector<3, Num>::Get(1, 1, 1));
#ifdef _DEBUG
  CAABB kAABB(vMin, vMax);
  COBB *pOBB = Cast<COBB>(kAABB.GetTransformed(kXForm));
  ASSERT(pOBB);
  ASSERT(IsEqual((vCenter - pOBB->m_vCenter).Length(), 0));
  for (i = 0; i < 3; i++)
    ASSERT(IsEqual((vExtent[i] - pOBB->m_vExtent[i]).Length(), 0));
#endif
}

CShape3D::Num COBB::CheckSidePoint(int iSide, int iPoint, CVector<4, Num> const (&vPlanes)[2],
                                   CVector<3, Num> const (&vPoints)[2][4], Num const (&nLen)[2][2],
                                   CVector<3, Num> const (&vBasis)[2][2])
{
  // Return the distance from a point to the other side's plane, if a point's projection is inside the other side
  CVector<3, Num> vP;
  Num nT, nDist;
  int iDim;
  vP = vPoints[iSide][iPoint] - vPoints[!iSide][0]; // Subtract other side's origin
  for (iDim = 0; iDim < 2; iDim++) {
    nT = vP % vBasis[!iSide][iDim];
    if (nT < 0 || nT > nLen[!iSide][iDim])
      return Util::F_INFINITY;
  }
  nDist = vPlanes[!iSide].x() * vPoints[iSide][iPoint].x() + vPlanes[!iSide].y() * vPoints[iSide][iPoint].y() +
          vPlanes[!iSide].z() * vPoints[iSide][iPoint].z() + vPlanes[!iSide].w();
  return nDist;
}

CShape3D::Num COBB::Side2SideDist(CVector<3, Num> const &vOrigin0, CVector<3, Num> const &vX0,
                                  CVector<3, Num> const &vY0, CVector<4, Num> const &vPlane0,
                                  CVector<3, Num> const &vOrigin1, CVector<3, Num> const &vX1,
                                  CVector<3, Num> const &vY1, CVector<4, Num> const &vPlane1)
{
  CVector<4, Num> vPlanes[2];
  CVector<3, Num> vPoints[2][4];
  CVector<3, Num> vBasis[2][2];
  Num nLen[2][2];
  Num nDist, nCurDist;
  int iEdge0, iEdge1;
  int iEdgeEnd0, iEdgeEnd1;

  vPlanes[0] = vPlane0;
  vPlanes[1] = vPlane1;

  vPoints[0][0] = vOrigin0;
  vPoints[0][1] = vOrigin0 + vX0;
  vPoints[0][2] = vOrigin0 + vX0 + vY0;
  vPoints[0][3] = vOrigin0 + vY0;

  vPoints[1][0] = vOrigin1;
  vPoints[1][1] = vOrigin1 + vX1;
  vPoints[1][2] = vOrigin1 + vX1 + vY1;
  vPoints[1][3] = vOrigin1 + vY1;

  nLen[0][0] = vX0.Length();
  nLen[0][1] = vY0.Length();
  nLen[1][0] = vX1.Length();
  nLen[1][1] = vY1.Length();

  vBasis[0][0] = vX0;
  vBasis[0][1] = vY0;
  vBasis[1][0] = vX1;
  vBasis[1][1] = vY1;

  nDist = Util::F_INFINITY;

  for (iEdge0 = 0; iEdge0 < 4; iEdge0++) {
    // Project edge end points on the other side's plane and if the projection's inside the side, take account for it in the distance
    iEdgeEnd0 = (iEdge0 + 1) % 4;
    nCurDist = CheckSidePoint(0, iEdge0, vPlanes, vPoints, nLen, vBasis);
    ASSERT(nCurDist >= 0 || !"Incorrect vX / vY winding, derived plane normal points in the wrong direction");
    if (nCurDist < nDist)
      nDist = nCurDist;

    for (iEdge1 = 0; iEdge1 < 4; iEdge1++) {
      iEdgeEnd1 = (iEdge1 + 1) % 4;
      nCurDist = CheckSidePoint(1, iEdge1, vPlanes, vPoints, nLen, vBasis);
      ASSERT(nCurDist >= 0 || !"Incorrect vX / vY winding, derived plane normal points in the wrong direction");
      if (nCurDist < nDist)
        nDist = nCurDist;

      // Calculate distance between each pair of edges on the two sides
      nCurDist = CLine3D::Dist2Line(vPoints[0][iEdge0], vPoints[0][iEdgeEnd0], 0, 1, vPoints[1][iEdge1], vPoints[1][iEdgeEnd1], 0, 1);
      if (nCurDist < nDist)
        nDist = nCurDist;
    }
  }
  return nDist;
}

void COBB::GetSideData(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3], int iSide,
                       CVector<3, Num> &vOrigin, CVector<3, Num> &vX, CVector<3, Num> &vY, CVector<4, Num> &vPlane)
{
  int iDim, iDimX, iDimY;
  CVector<3, Num> vNorm;

  ASSERT(iSide >= 0 && iSide < 6);

  iDim = iSide >> 1;
  iDimX = (iDim + 1) % 3;
  iDimY = (iDim + 2) % 3;

  if (iSide & 1) {
    vOrigin = vCenter + vExtent[iDim];
    vOrigin -= vExtent[iDimX];
    vOrigin -= vExtent[iDimY];
    vX = vExtent[iDimX] * 2;
    vY = vExtent[iDimY] * 2;
    vNorm = vExtent[iDim];
  } else {
    vOrigin = vCenter - vExtent[iDim];
    vOrigin += vExtent[iDimX];
    vOrigin -= vExtent[iDimY];
    vX = vExtent[iDimX] * -2;
    vY = vExtent[iDimY] * 2;
    vNorm = -vExtent[iDim];
  }
  vNorm.Normalize();
  vPlane.Set(vNorm);
  vPlane.w() = -(vOrigin % vNorm);
  ASSERT(ID(CONCAT(CVector<4, Num>::Get(vCenter.x(), vCenter.y(), vCenter.z(), 1) % vPlane < 0)));
}

CShape3D::Num COBB::Dist2OBB(const CVector<3, Num> &vCenter0, const CVector<3, Num> (&vExtent0)[3],
                             const CVector<3, Num> &vCenter1, const CVector<3, Num> (&vExtent1)[3])
{
  CVector<3, Num> vPoints[2][8];
  int i, j, iPoint, iDividingBox;
  int iDividingSide[2];
  CVector<3, Num> vO[2], vX[2], vY[2];
  CVector<4, Num> vPlane[2];
  Num nDist, nCurDist;

  for (i = 0; i < 8; i++) {
    vPoints[0][i] = vCenter0;
    vPoints[1][i] = vCenter1;
    for (j = 0; j < 3; j++)
      if (i & (1 << j)) {
        vPoints[0][i] += vExtent0[j];
        vPoints[1][i] += vExtent1[j];
      } else {
        vPoints[0][i] -= vExtent0[j];
        vPoints[1][i] -= vExtent1[j];
      }
  }

  // Check which side of each box is dividing the boxes from each other
  // The closest point on a box will belong to a dividing side
  // If no sides are dividing, the boxes intersect
  for (i = 0; i < 2; i++) {
    const CVector<3, Num> &vCenter = i ? vCenter1 : vCenter0;
    const CVector<3, Num> (&vExtent)[3] = *(const CVector<3, Num> (*)[3]) (i ? vExtent1 : vExtent0);
    iDividingSide[i] = -1;
    for (j = 0; j < 6; j++) {
      GetSideData(vCenter, vExtent, j, vO[i], vX[i], vY[i], vPlane[i]);
      for (iPoint = 0; iPoint < 8; iPoint++) {
        CVector<3, Num> &vPt = vPoints[!i][iPoint];
        nCurDist = vPt.x() * vPlane[i].x() + vPt.y() * vPlane[i].y() + vPt.z() * vPlane[i].z() + vPlane[i].w();
        if (nCurDist <= 0)
          break;
      }
      if (iPoint >= 8) { // All the other box's points are on the other side of the plane, so this plane is dividing
        iDividingSide[i] = j;
        break;
      }
    }
  }
  if (iDividingSide[0] < 0 && iDividingSide[1] < 0) // No dividing sides, boxes are intersecting
    return 0;
  if (iDividingSide[0] >= 0)
    iDividingBox = 0;
  else
    iDividingBox = 1;

  if (iDividingSide[!iDividingBox] >= 0) { // Both boxes have dividing sides so only they need to be checked against each other
    nDist = Side2SideDist(vO[0], vX[0], vY[0], vPlane[0], vO[1], vX[1], vY[1], vPlane[1]);
    return nDist;
  }

  const CVector<3, Num> &vCenter = !iDividingBox ? vCenter1 : vCenter0;
  const CVector<3, Num> (&vExtent)[3] = *(const CVector<3, Num> (*)[3]) (!iDividingBox ? vExtent1 : vExtent0);
  CVector<3, Num> vDivNorm;
  nDist = Util::F_INFINITY;
  vDivNorm.Set(vPlane[iDividingBox]);
  for (j = 0; j < 6; j++) {
    GetSideData(vCenter, vExtent, j, vO[!iDividingBox], vX[!iDividingBox], vY[!iDividingBox], vPlane[!iDividingBox]);
    CVector<3, Num> vNorm;
    vNorm.Set(vPlane[!iDividingBox]);
    if (vDivNorm % vNorm >= 0) // Side is facing in the same direction as the dividing plane, i.e. AWAY from the box with the dividing side, ignore it
      continue;
    nCurDist = Side2SideDist(vO[0], vX[0], vY[0], vPlane[0], vO[1], vX[1], vY[1], vPlane[1]);
    if (nCurDist < nDist)
      nDist = nCurDist;
  }

  return nDist;
}

CShape3D::Num COBB::Dist2AABB(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                              const CVector<3, Num> &vMin, const CVector<3, Num> &vMax)
{
  // It is cheaper to convert an AABB to an OBB than the other way around, that's why we do so
  CVector<3, Num> vCenter1, vExtent1[3];
  int i;
  Num nDist;
  vCenter1 = (vMin + vMax) * 0.5;
  for (i = 0; i < 3; i++) {
    vExtent1[i].SetZero();
    vExtent1[i][i] = (vMax[i] - vMin[i]) / 2;
  }
  nDist = Dist2OBB(vCenter, vExtent, vCenter1, vExtent1);
  return nDist;
}

CShape3D::Num COBB::Dist2Sphere(const CVector<3, Num> &vOBBCenter, const CVector<3, Num> (&vExtent)[3],
                                const CVector<3, Num> &vSphereCenter, Num nRadius)
{
  Num nDist;
  CVector<3, Num> vMin, vMax;
  CXForm kXForm, kXFormInv;
  CVector<3, Num> vXCenter;
  Num nXRadius;
  Untransform(vOBBCenter, vExtent, vMin, vMax, kXForm);
  kXForm.m_pMatrix->GetInverse(*kXFormInv.m_pMatrix);
  CSphere::Transform(vSphereCenter, nRadius, kXFormInv, vXCenter, nXRadius);
  nDist = CAABB::Dist2Sphere(vMin, vMax, vXCenter, nXRadius);
  return nDist;
}

CShape3D::Num COBB::Dist2Plane(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3], const CVector<4, Num> &vPlane, bool bHalfSpace)
{
  Num nDist;
  CVector<3, Num> vMin, vMax;
  CXForm kXForm, kXFormInv;
  CVector<4, Num> vXPlane;
  Untransform(vCenter, vExtent, vMin, vMax, kXForm);
  kXForm.m_pMatrix->GetInverse(*kXFormInv.m_pMatrix);
  CPlane::Transform(vPlane, kXFormInv, vXPlane);
  nDist = CAABB::Dist2Plane(vMin, vMax, vXPlane, bHalfSpace);
  return nDist;
}

CShape3D::Num COBB::Dist2Line(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                              const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax)
{
  Num nDist;
  CVector<3, Num> vMin, vMax;
  CXForm kXForm, kXFormInv;
  CVector<3, Num> vXPoint0, vXPoint1;
  Untransform(vCenter, vExtent, vMin, vMax, kXForm);
  kXForm.m_pMatrix->GetInverse(*kXFormInv.m_pMatrix);
  CLine3D::Transform(vPoint0, vPoint1, kXFormInv, vXPoint0, vXPoint1);
  nDist = CAABB::Dist2Line(vMin, vMax, vXPoint0, vXPoint1, nLineMin, nLineMax);
  return nDist;
}

CShape3D::Num COBB::Dist2Point(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3], const CVector<3, Num> &vPoint)
{
  Num nDist;
  CVector<3, Num> vMin, vMax;
  CXForm kXForm, kXFormInv;
  CVector<3, Num> vXPoint;
  Untransform(vCenter, vExtent, vMin, vMax, kXForm);
  kXForm.m_pMatrix->GetInverse(*kXFormInv.m_pMatrix);
  CPoint3D::Transform(vPoint, kXFormInv, vXPoint);
  nDist = CAABB::Dist2Point(vMin, vMax, vXPoint);
  return nDist;
}
