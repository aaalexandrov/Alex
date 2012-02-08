#include "stdafx.h"
#include "Convex.h"

IMPRTTI(CConvex, CObject)

CConvex::CConvex()
{
}

CConvex::~CConvex()
{
}

void CConvex::Reset()
{
  m_arrPlanes.SetCount(0);
  m_arrVertices.SetCount(0);
  m_arrEdges.SetCount(0);
}

bool CConvex::AddPlane(CVector<4, Num> const &vPlane)
{
  // Theoretically here we might check for redundant planes and not add them
  m_arrPlanes.Append(vPlane);
  return true;
}

bool CConvex::InitFromPoints(CVector<3, Num> const *pPoints, UINT uiCount)
{
  int i, iMin, iMinAngle;
  CVector<3, Num> vNormal, vDelta;
  Num nMinDot, nDot;

  ASSERT(!m_arrPlanes.m_iCount);
  ASSERT(!m_arrVertices.m_iCount);
  ASSERT(!m_arrEdges.m_iCount);
  ASSERT(pPoints && uiCount >= 3);

  // Find the point with minimum x, it's part of the convex hull
  iMin = 0;
  for (i = 1; i < (int) uiCount; i++)
    if (pPoints[i].x() < pPoints[iMin].x())
      iMin = i;
  // Find the edge that passes through the minimum x point and any other point in the set that is at minimum angle with the x = 0 plane
  // That edge belongs to the convex hull
  vNormal.Set(1, 0, 0);
  iMinAngle = !iMin;
  nMinDot = (pPoints[iMinAngle] - pPoints[iMin]).GetNormalized() % vNormal;
  for (i = 0; i < (int) uiCount; i++) {
    if (i == iMin || i == iMinAngle)
      continue;
    vDelta = pPoints[i] - pPoints[iMin];
    vDelta.Normalize();
    nDot = vDelta % vNormal;
    ASSERT(nDot >= 0);
    if (nDot < nMinDot) {
      iMinAngle = i;
      nMinDot = nDot;
    }
  }
  CArray<CArray<int> > arrPlaneVertexIncidence;
  CArray<int> arrVertex2Point;
  int iMinInd, iMinAngleInd, iEdge, j, iNewInd, iNewEdge;
  CVector<4, Num> vPlane;
  CVector<3, Num> vEdge, vEdgeNew;

  iMinInd = AddVertex(pPoints[iMin]);
  iMinAngleInd = AddVertex(pPoints[iMinAngle]);
  arrVertex2Point.SetCount(m_arrVertices.m_iCount);
  arrVertex2Point[iMinInd] = iMin;
  arrVertex2Point[iMinAngleInd] = iMinAngle;
  AddEdge(iMinInd, iMinAngleInd);
  while (1) {
    // Find an edge that whose second incidental plane hasn't been found yet
    for (iEdge = m_arrEdges.m_iCount - 1; iEdge >= 0; iEdge--)
      if (m_arrEdges[iEdge].iPlane0 < 0 || m_arrEdges[iEdge].iPlane1 < 0)
        break;
    if (iEdge < 0) // Every edge is incidental to two planes -> the convex hull is built
      break;
    vEdge = m_arrVertices[m_arrEdges[iEdge].iVert1] - m_arrVertices[m_arrEdges[iEdge].iVert0];
    // Make a plane through this edge and any other valid point in the set and check if it belongs to the convex hull
    for (i = 0; i < (int) uiCount; i++) {
      if (i == arrVertex2Point[m_arrEdges[iEdge].iVert0] || i == arrVertex2Point[m_arrEdges[iEdge].iVert1])
        continue;
      if (m_arrEdges[iEdge].iPlane0) {
        CArray<int> &arrVertices = arrPlaneVertexIncidence[m_arrEdges[iEdge].iPlane0];
        for (j = 0; j < arrVertices.m_iCount; j++)
          if (i == arrVertex2Point[arrVertices[j]])
            break;
        if (j < arrVertices.m_iCount) // Point is incident to the plane this edge belongs to, we need another point
          continue;
      }
      vEdgeNew = pPoints[i] - m_arrVertices[m_arrEdges[iEdge].iVert0];
      vNormal = vEdge ^ vEdgeNew;
      if (IsEqual(vNormal.LengthSqr(), 0)) // The edge + the new point don't make a valid plane
        continue;
      vNormal.Normalize();
      vPlane.Set(vNormal);
      vPlane.w() = -(vNormal % m_arrVertices[m_arrEdges[iEdge].iVert0]);
      bool bInverted = false;
      for (j = 0; j < (int) uiCount; j++) {
        Num nDist = CPlane::CalcValue(vPlane, pPoints[j]);
        if (nDist > 0) {
          if (bInverted)
            break;
          vPlane *= -1;
          bInverted = true;
        }
      }
      if (j >= (int) uiCount) { // All points are on one side of the new plane, add it to the convex hull
        iNewInd = AddVertex(pPoints[i]);
        m_arrPlanes.Append(vPlane);
        AddEdgePlane(iEdge, m_arrPlanes.m_iCount - 1);
        iNewEdge = AddEdge(m_arrEdges[iEdge].iVert0, iNewInd);
        AddEdgePlane(iNewEdge, m_arrPlanes.m_iCount - 1);
        iNewEdge = AddEdge(m_arrEdges[iEdge].iVert1, iNewInd);
        AddEdgePlane(iNewEdge, m_arrPlanes.m_iCount - 1);
        break;
      }
    }
    ASSERT(i < (int) uiCount); // Couldn't find a second plane that an edge is incident to, are the input points all on a single plane?
    if (i >= (int) uiCount) 
      return false;
  }
  return true;
}

void CConvex::InitData()
{
  int i, j, k;
  CVector<3, Num> vP0, vP1, vNorm, vLine, vVert0, vVert1;
  Num nMin, nMax;
  Num nT;

  ASSERT(!m_arrVertices.m_iCount);
  ASSERT(!m_arrEdges.m_iCount);

  // Find all edges and record all vertices
  for (i = 1; i < m_arrPlanes.m_iCount; i++) {
    for (j = 0; j < i; j++) {
      if (!CPlane::IntersectPlane(m_arrPlanes[i], m_arrPlanes[j], vP0, vP1))
        continue;
      nMin = Util::F_NEG_INFINITY;
      nMax = Util::F_INFINITY;
      vLine = vP1 - vP0;
      // Prune the line against all other planes' negative half-spaces
      for (k = 0; k < m_arrPlanes.m_iCount; k++) {
        if (k == i || k == j)
          continue;
        nT = CPlane::GetIntersectionFactor(m_arrPlanes[k], vP0, vLine);
        vNorm.Set(m_arrPlanes[k]);
        if (vNorm % vLine < 0) { // The negative side of the plane occupies parts of the line with t < nT
          if (nMax < nT)
            break;
          if (nMin < nT)
            nMin = nT;
        } else { // Negative side of the plane occupies parts of the line with t > nT
          if (nMin > nT)
            break;
          if (nMax > nT)
            nMax = nT;
        }
      }
      if (k < m_arrPlanes.m_iCount) // No part of these two planes' intersection lies within the convex, i.e. this is not an edge
        break;
      m_arrEdges.SetCount(m_arrEdges.m_iCount + 1);
      TEdge &kEdge = m_arrEdges[m_arrEdges.m_iCount - 1];
      if (nMin == Util::F_NEG_INFINITY)
        if (nMax == Util::F_INFINITY) {
          vVert0 = vP0;
          vVert1 = vP1;
        } else {
          vVert1 = vP0 + vLine * nMax;
          vVert0 = vVert1 - vLine;
          nMax = 1;
        }
      else
        if (nMax == Util::F_INFINITY) {
          vVert0 = vP0 + vLine * nMin;
          vVert1 = vVert0 + vLine;
          nMin = 0;
        } else {
          vVert0 = vP0 + vLine * nMin;
          vVert1 = vP0 + vLine * nMax;
          nMin = 0;
          nMax = 1;
        }
      kEdge.iVert0 = AddVertex(vVert0);
      kEdge.iVert1 = AddVertex(vVert1);
      kEdge.nMin = nMin;
      kEdge.nMax = nMax;
      kEdge.iPlane0 = j;
      kEdge.iPlane1 = i;
    }
  }
}

int CConvex::AddVertex(CVector<3, Num> const &vVertex)
{
  int i;
  Num nDist;
  for (i = 0; i < m_arrVertices.m_iCount; i++) {
    nDist = (m_arrVertices[i] - vVertex).LengthSqr();
    if (IsEqual(nDist, 0))
      return i;
  }
  m_arrVertices.Append(vVertex);
  return i;
}

int CConvex::AddEdge(int iVert0, int iVert1)
{
  int i;
  ASSERT(iVert0 != iVert1);
  if (iVert0 > iVert1) 
    Util::Swap(iVert0, iVert1);
  for (i = 0; i < m_arrEdges.m_iCount; i++)
    if (m_arrEdges[i].iVert0 == iVert0 && m_arrEdges[i].iVert1 == iVert1)
      break;
  if (i >= m_arrEdges.m_iCount) {
    m_arrEdges.SetCount(m_arrEdges.m_iCount + 1);
    m_arrEdges[i].iVert0 = iVert0;
    m_arrEdges[i].iVert1 = iVert1;
    m_arrEdges[i].nMin = 0;
    m_arrEdges[i].nMax = 1;
    m_arrEdges[i].iPlane0 = -1;
    m_arrEdges[i].iPlane1 = -1;
  }
  return i;
}

void CConvex::AddEdgePlane(int iEdge, int iPlane)
{
  if (m_arrEdges[iEdge].iPlane0 < 0)
    m_arrEdges[iEdge].iPlane0 = iPlane;
  else
    m_arrEdges[iEdge].iPlane1 = iPlane;
}

bool CConvex::Intersects(CShape3D const *pShape, bool bPrecise)
{
  int i;
  Num nDist;
  CPlane kPlane;
  static CArray<int> kActivePlanes; // if called frequently, memory allocation here can cause memory manager lock contingency so we make the array static to avoid the allocation

  kActivePlanes.SetCount(0);
  // Check if any of the convex sides divides the shape from the convex
  for (i = 0; i < m_arrPlanes.m_iCount; i++) {
    kPlane.m_vPlane = m_arrPlanes[i];
    nDist = kPlane.GetDistance(pShape);
    if (nDist > 0)
      return false;
    if (nDist < 0)
      continue;
    if (!bPrecise)
      continue;
    kActivePlanes.Append(i);
  }

  if (!bPrecise)
    return true;

  CPoint3D const *pPoint = Cast<CPoint3D>(pShape);
  if (pPoint)
    return true;
  CLine3D const *pLine = Cast<CLine3D>(pShape);
  if (pLine) 
    return Intersects(pLine, kActivePlanes);
  CPlane const *pPlane = Cast<CPlane>(pShape);
  if (pPlane)
    return Intersects(pPlane);
  CSphere const *pSphere = Cast<CSphere>(pShape);
  if (pSphere)
    return Intersects(pSphere, kActivePlanes);
  CAABB const *pAABB = Cast<CAABB>(pShape);
  if (pAABB)
    return Intersects(pAABB);
  COBB const *pOBB = Cast<COBB>(pShape);
  if (pOBB)
    return Intersects(pOBB);

  ASSERT(!"Precisely intersecting convex with an unknown shape");

  return true;
}

bool CConvex::Intersects(CLine3D const *pLine, CArray<int> const &kActivePlanes)
{
  CVector<3, Num> vPoint;
  int i;

  for (i = 0; i < kActivePlanes.m_iCount; i++) {
    if (!CPlane::IntersectLine(m_arrPlanes[kActivePlanes[i]], pLine->m_vPoints[0], pLine->m_vPoints[1], pLine->GetMinLimit(), pLine->GetMaxLimit(), vPoint))
      continue;
    if (PointInside(vPoint, i))
      return true;
  }
  // Line doesn't directly intersect any side so it's either all inside or outside
  // We already know it's intersecting a plane but no intersection point is inside
  // So a point on the line is outside the convex -> all of the line is outside
  ASSERT(!PointInside(pLine->GetCenter(), -1));
  return false;
}

bool CConvex::Intersects(CPlane const *pPlane)
{
  int i;
  Num nEqVal;
  bool bHalfSpace, bPositive, bNegative;

  bHalfSpace = pPlane->IsHalfSpace();
  if (!bHalfSpace)
    bPositive = bNegative = false;
  for (i = 0; i < m_arrVertices.m_iCount; i++) {
    nEqVal = CPlane::CalcValue(pPlane->m_vPlane, m_arrVertices[i]);
    if (bHalfSpace) {
      if (nEqVal <= 0)
        return true;
    } else {
      if (nEqVal > 0)
        bPositive = true;
      else
        if (nEqVal < 0)
          bNegative = true;
        else 
          return true;
      if (bPositive & bNegative)
        return true;
    }
  }
  return false;
}

bool CConvex::Intersects(CSphere const *pSphere, CArray<int> const &kActivePlanes)
{
  int i, iEdge, iOtherPlane;
  Num nDist, nEqVal, nEdgeDist;
  CVector<3, Num> vPoint, vNorm;
  bool bPointIntoSide;

  for (i = 0; i < kActivePlanes.m_iCount; i++) {
    CVector<4, Num> const &vPlane = m_arrPlanes[kActivePlanes[i]];
    nDist = CPlane::Dist2Point(vPlane, false, pSphere->m_vCenter);
    ASSERT(abs(nDist) <= pSphere->m_nRadius);
    vNorm.Set(vPlane);
    vNorm.Normalize();
    vPoint = vNorm * -nDist;
    ASSERT(IsEqual(CPlane::CalcValue(vPlane, vPoint), 0));
    bPointIntoSide = true;
    for (iEdge = 0; iEdge < m_arrEdges.m_iCount; iEdge++) {
      TEdge const &kEdge = m_arrEdges[iEdge];
      if (kEdge.iPlane0 == kActivePlanes[i])
        iOtherPlane = kEdge.iPlane1;
      else
        if (kEdge.iPlane1 == kActivePlanes[i])
          iOtherPlane = kEdge.iPlane0;
        else
          continue;
      nEqVal = CPlane::CalcValue(m_arrPlanes[iOtherPlane], vPoint);
      if (nEqVal >= 0)
        bPointIntoSide = false;
      nEdgeDist = CSphere::Dist2Line(pSphere->m_vCenter, pSphere->m_nRadius, 
                                     m_arrVertices[kEdge.iVert0], m_arrVertices[kEdge.iVert1], kEdge.nMin, kEdge.nMax);
      if (IsEqual(nEdgeDist, 0)) // Sphere intersects an edge of the convex
        return true;
    }
    if (bPointIntoSide) // Projection of the sphere's center on the plane is inside the convex side
      return true;
  }
  return false;
}

bool CConvex::Intersects(CAABB const *pAABB)
{
  int i, iPointsOnSide;

  iPointsOnSide = 0x3f;

  for (i = 0; i < m_arrVertices.m_iCount; i++) {
    CVector<3, Num> const &vP = m_arrVertices[i];
    if (vP.x() >= pAABB->m_vMin.x())
      iPointsOnSide &= ~1;
    if (vP.x() <= pAABB->m_vMax.x())
      iPointsOnSide &= ~2;
    if (vP.y() >= pAABB->m_vMin.y())
      iPointsOnSide &= ~4;
    if (vP.y() <= pAABB->m_vMax.y())
      iPointsOnSide &= ~8;
    if (vP.z() >= pAABB->m_vMin.z())
      iPointsOnSide &= ~16;
    if (vP.z() <= pAABB->m_vMax.z())
      iPointsOnSide &= ~32;
    if (!iPointsOnSide) // None of the sides of the AABB is dividing the convex from the AABB, so they intersect
      return true;
  }
  return false;
}

bool CConvex::Intersects(COBB const *pOBB)
{
  int i, j;
  CVector<3, Num> vOrigin, vX, vY;
  CVector<4, Num> vPlane;
  Num nDist;

  for (i = 0; i < 6; i++) {
    pOBB->GetSideData(pOBB->m_vCenter, pOBB->m_vExtent, i, vOrigin, vX, vY, vPlane);
    for (j = 0; j < m_arrVertices.m_iCount; j++) {
      nDist = CPlane::CalcValue(vPlane, m_arrVertices[j]);
      if (nDist <= 0)
        break;
    }
    if (j >= m_arrVertices.m_iCount) // Found a dividing side
      return false;
  }
  return true;
}

bool CConvex::PointInside(CVector<3, Num> const &vPoint, int iExcludePlane)
{
  int i;
  Num nEqVal;

  for (i = 0; i < m_arrPlanes.m_iCount; i++) {
    if (i == iExcludePlane)
      continue;
    nEqVal = CPlane::CalcValue(m_arrPlanes[i], vPoint);
    if (nEqVal > 0)
      return false;
  }

  return true;
}
