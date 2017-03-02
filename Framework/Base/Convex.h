#ifndef __CONVEX_H
#define __CONVEX_H

#include "Array.h"
#include "Shape.h"

class CConvex: public CObject {
  DEFRTTI(CConvex, CObject, true)
public:
  typedef CShape3D::Num Num;

  struct TEdge {
    int iVert0, iVert1;
    Num nMin, nMax;
    int iPlane0, iPlane1;
  };

  CArray<CVector<4, Num> > m_arrPlanes;
  CArray<CVector<3, Num> > m_arrVertices;
  CArray<TEdge>            m_arrEdges;

  CConvex();
  ~CConvex();

  void Reset();
  bool AddPlane(CVector<4, Num> const &vPlane);
  // Initialize to the convex hull of a set of points
  bool InitFromPoints(CVector<3, Num> const *pPoints, UINT uiCount);

  void InitData();
  int AddVertex(CVector<3, Num> const &vVertex);
  int AddEdge(int iVert0, int iVert1);
  void AddEdgePlane(int iEdge, int iPlane);

  bool Intersects(CShape3D const *pShape, bool bPrecise);

  bool Intersects(CLine3D const *pLine, CArray<int> const &kActivePlanes);
  bool Intersects(CPlane const *pPlane);
  bool Intersects(CSphere const *pSphere, CArray<int> const &kActivePlanes);
  bool Intersects(CAABB const *pAABB);
  bool Intersects(COBB const *pOBB);

  bool PointInside(CVector<3, Num> const &vPoint, int iExcludePlane);

  static inline Num IsEqual(Num n, Num m, Num nEpsilon = 0.0005) { return Util::abs(m - n) <= nEpsilon; }
};

#endif
