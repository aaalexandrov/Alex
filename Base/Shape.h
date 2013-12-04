#ifndef __SHAPE_H
#define __SHAPE_H

#include "Base.h"
#include "Util.h"
#include "Vector.h"
#include "Matrix.h"
#include "Transform.h"

class CShape3D: public CObject {
  DEFRTTI(CShape3D, CObject, false)
public:
  typedef float Num;
  static const Num INVALID;

  virtual ~CShape3D() {}

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount) = 0;
  virtual void CopyFrom(CShape3D const *pSrc) = 0;
  virtual bool IsEmpty() const = 0;
  virtual void SetEmpty() = 0;
  virtual void IncludePoint(CVector<3> const &v) = 0;
  virtual Num GetVolume() const = 0;
  virtual CVector<3, Num> GetCenter() const = 0;
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const = 0;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const = 0;

  virtual Num GetDistance(const CShape3D *pShape) const;
  virtual CShape3D *Clone() const;

  static inline Num IsEqual(Num n, Num m, Num nEpsilon = 0.0005) { return abs(m - n) <= nEpsilon; }
  static inline bool IsInvalid(Num n) { return *(uint32_t *) &n == Util::s_dwQNAN; }
};

class CPoint3D: public CShape3D {
  DEFRTTI(CPoint3D, CShape3D, true)
public:
  CVector<3, Num> m_vPoint;

  CPoint3D() { SetEmpty(); }
  CPoint3D(const CVector<3, Num> &vPoint) { Init(vPoint); }
  virtual ~CPoint3D() {}

  void Init(const CVector<3, Num> &vPoint) { m_vPoint = vPoint; }

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount);
  virtual void CopyFrom(CShape3D const *pSrc);
  virtual bool IsEmpty() const;
  virtual void SetEmpty();
  virtual void IncludePoint(CVector<3> const &v) { ASSERT(!"Incremental building of points not supported"); }
  virtual Num GetVolume() const;
  virtual CVector<3, Num> GetCenter() const { return m_vPoint; }
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const;

  static void Transform(CVector<3, Num> const &vPoint, CXForm const &kXForm, CVector<3, Num> &vXPoint);

  static Num Dist2Point(const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1);
};

class CLine3D: public CShape3D {
  DEFRTTI(CLine3D, CShape3D, true)
public:
  CVector<3, Num> m_vPoints[2];

  CLine3D() { SetEmpty(); }
  CLine3D(const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1) { Init(vPoint0, vPoint1); }
  virtual ~CLine3D() {}

  void Init(const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1) { m_vPoints[0] = vPoint0; m_vPoints[1] = vPoint1; }

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount);
  virtual void CopyFrom(CShape3D const *pSrc);
  virtual bool IsEmpty() const;
  virtual void SetEmpty();
  virtual void IncludePoint(CVector<3> const &v) { ASSERT(!"Incremental building of lines not supported"); }
  virtual Num GetVolume() const;
  virtual CVector<3, Num> GetCenter() const { return (m_vPoints[0] + m_vPoints[1]) * 0.5; }
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const;

  // Min / Max values of the parameter T for points on the valid part of the line if the line parametrized as P = (1-T)*P0 + T*P1
  // By overriding these functions, subclasses can get the behavior of segments and rays as well as an infinite line
  virtual Num GetMinLimit() const { return Util::F_NEG_INFINITY; }
  virtual Num GetMaxLimit() const { return Util::F_INFINITY; }

  static void Transform(CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1, CXForm const &kXForm,
                        CVector<3, Num> &vXPoint0, CVector<3, Num> &vXPoint1);

  static Num Dist2Line(const CVector<3, Num> &vLine0Point0, const CVector<3, Num> &vLine0Point1, Num nLine0Min, Num nLine0Max,
                       const CVector<3, Num> &vLine1Point0, const CVector<3, Num> &vLine1Point1, Num nLine1Min, Num nLine1Max);
  static Num Dist2Point(const CVector<3, Num> &vLinePoint0, const CVector<3, Num> &vLinePoint1, Num nLineMin, Num nLineMax,
                        const CVector<3, Num> &vPoint);
};

class CSegment3D: public CLine3D {
  DEFRTTI(CSegment3D, CLine3D, true)
public:
  CSegment3D(): CLine3D() {}
  CSegment3D(const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1): CLine3D(vPoint0, vPoint1) {}

  virtual Num GetMinLimit() const { return 0; }
  virtual Num GetMaxLimit() const { return 1; }
};

class CRay3D: public CLine3D {
  DEFRTTI(CRay3D, CLine3D, true)
public:
  CRay3D(): CLine3D() {}
  CRay3D(const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1): CLine3D(vPoint0, vPoint1) {}

  virtual Num GetMinLimit() const { return 0; }
  virtual Num GetMaxLimit() const { return Util::F_INFINITY; }
};

class CPlane: public CShape3D { // A plane shape occupies the points directly on the plane
  DEFRTTI(CPlane, CShape3D, true)
public:
  CVector<4, Num> m_vPlane;

  CPlane() { SetEmpty(); }
  CPlane(const CVector<4, Num> &vPlane) { Init(vPlane); }
  virtual ~CPlane() {}

  void Init(const CVector<4, Num> &vPlane) { m_vPlane = vPlane; }

  virtual bool IsHalfSpace() const { return false; }

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount);
  virtual void CopyFrom(CShape3D const *pSrc);
  virtual bool IsEmpty() const;
  virtual void SetEmpty();
  virtual void IncludePoint(CVector<3> const &v) { ASSERT(!"Incremental building of planes not supported"); }
  virtual Num GetVolume() const;
  virtual CVector<3, Num> GetCenter() const  { return GetPoint(m_vPlane); }
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const;

  static void Transform(CVector<4, Num> const &vPlane, CXForm const &kXForm, CVector<4, Num> &vXPlane);

  static inline Num CalcValue(CVector<4, Num> const &vPlane, CVector<3, Num> const &vPoint)
    { return vPoint.x() * vPlane.x() + vPoint.y() * vPlane.y() + vPoint.z() * vPlane.z() + vPlane.w(); }

  static inline CVector<4, Num> CalcFromPoints(CVector<3, Num> const &v0, CVector<3, Num> const &v1, CVector<3, Num> const &v2)
    { CVector<3, Num> vNormal = ((v1 - v0) ^ (v2 - v0)).GetNormalized(); return CVector<4, Num>::Get(vNormal.x(), vNormal.y(), vNormal.z(), -(vNormal % v0)); }

  // Get the point on the plane that's closest to (0, 0, 0)
  static CVector<3, Num> GetPoint(const CVector<4, Num> &vPlane);
  // Get value of parameter t of the intersection point P between plane and line, where P = vOrigin + t * vDirection
  static Num GetIntersectionFactor(CVector<4, Num> const &vPlane, CVector<3, Num> const &vOrigin, CVector<3, Num> const &vDirection);
  static bool Intersect2Planes(CVector<4, Num> const &vPlane0, CVector<4, Num> const &vPlane1,
                               CVector<4, Num> const &vPlane2, CVector<3, Num> &vPoint);
  static bool IntersectPlane(CVector<4, Num> const &vPlane0, CVector<4, Num> const &vPlane1,
                             CVector<3, Num> &vLine0, CVector<3, Num> &vLine1);
  static bool IntersectLine(CVector<4, Num> const &vPlane, CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1,
                            Num nLineMin, Num nLineMax, CVector<3, Num> &vPoint);

  static Num Dist2Plane(const CVector<4, Num> &vPlane0, bool bHalfSpace0, const CVector<4, Num> &vPlane1, bool bHalfSpace1);
  static Num Dist2Line(const CVector<4, Num> &vPlane, bool bHalfSpace,
                       const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax);
  static Num Dist2Point(const CVector<4, Num> &vPlane, bool bHalfSpace, const CVector<3, Num> &vPoint);
};

class CHalfSpace: public CPlane { // A half space shape occupies the all points on the negative side of the defining plane
  DEFRTTI(CHalfSpace, CPlane, true)
public:
  CHalfSpace(): CPlane() {}
  CHalfSpace(CVector<4, Num> const &vPlane): CPlane(vPlane) {}

  virtual bool IsHalfSpace() { return true; }
};

class CSphere: public CShape3D {
  DEFRTTI(CSphere, CShape3D, true)
public:
  CVector<3, Num> m_vCenter;
  Num m_nRadius;

  CSphere() { SetEmpty(); }
  CSphere(const CVector<3, Num> &vCenter, Num nRadius) { Init(vCenter, nRadius); }
  virtual ~CSphere() {}

  void Init(const CVector<3, Num> &vCenter, Num nRadius) { m_vCenter = vCenter; m_nRadius = nRadius; }

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount);
  virtual void CopyFrom(CShape3D const *pSrc);
  virtual bool IsEmpty() const;
  virtual void SetEmpty();
  virtual void IncludePoint(CVector<3> const &v);
  virtual Num GetVolume() const;
  virtual CVector<3, Num> GetCenter() const { return m_vCenter; }
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const;

  static void Transform(CVector<3, Num> const &vCenter, Num nRadius, CXForm const &kXForm,
                        CVector<3, Num> &vXCenter, Num &nXRadius);

  static bool IntersectLine(CVector<3, Num> const &vCenter, Num nRadius,
                            CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1,
                            Num nLineMin, Num nLineMax, Num &nMinFactor, Num &nMaxFactor);

  static Num Dist2Sphere(const CVector<3, Num> &vCenter0, Num nRadius0, const CVector<3, Num> &vCenter1, Num nRadius1);
  static Num Dist2Plane(const CVector<3, Num> &vCenter, Num nRadius, const CVector<4, Num> &vPlane, bool bHalfSpace);
  static Num Dist2Line(const CVector<3, Num> &vCenter, Num nRadius,
                       const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax);
  static Num Dist2Point(const CVector<3, Num> &vCenter, Num nRadius, const CVector<3, Num> &vPoint);
};

class CAABB: public CShape3D {
  DEFRTTI(CAABB, CShape3D, true)
public:
  CVector<3, Num> m_vMin, m_vMax;

  CAABB() { SetEmpty(); }
  CAABB(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax) { Init(vMin, vMax); }
  CAABB(CAABB const &kAABB) { Init(kAABB.m_vMin, kAABB.m_vMax); }
  CAABB(CSphere const &kSphere) { Init(kSphere); }
  virtual ~CAABB() {}

  void Init(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax) { m_vMin = vMin; m_vMax = vMax; }
  void Init(CSphere const &kSphere) { Init(kSphere.m_vCenter - kSphere.m_nRadius, kSphere.m_vCenter + kSphere.m_nRadius); }

  void IncludeAABB(CAABB const &kAABB);

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount);
  virtual void CopyFrom(CShape3D const *pSrc);
  virtual bool IsEmpty() const;
  virtual void SetEmpty();
  virtual void IncludePoint(CVector<3> const &v);
  virtual Num GetVolume() const;
  virtual CVector<3, Num> GetCenter() const { return (m_vMin + m_vMax) * 0.5; }
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const;

  static void Transform(CVector<3, Num> const &vMin, CVector<3, Num> const &vMax, CXForm const &kXForm,
                        CVector<3, Num> &vXCenter, CVector<3, Num> (&vXExtent)[3]);
  static void Transform(CVector<3, Num> const &vMin, CVector<3, Num> const &vMax, CXForm const &kXForm,
                        CVector<3, Num> &vXMin, CVector<3, Num> &vXMax);

  static bool TestLineCoordinate(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                                 const CVector<3, Num> &vOrigin, const CVector<3, Num> &vDirection,
                                 Num nLineMin, Num nLineMax, Num nCoordinate, int iDim, Num &nFactor);
  static Num Edge2LineDist(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax, int iDim, int iNumEdge,
                           const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax);

  static bool IntersectLine(CVector<3, Num> const &vMin, CVector<3, Num> const &vMax,
                            CVector<3, Num> const &vPoint0, CVector<3, Num> const &vPoint1,
                            Num nLineMin, Num nLineMax, Num &nMinFactor, Num &nMaxFactor);

  static Num Dist2AABB(const CVector<3, Num> &vMin0, const CVector<3, Num> &vMax0,
                       const CVector<3, Num> &vMin1, const CVector<3, Num> &vMax1);
  static Num Dist2Sphere(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                         const CVector<3, Num> &vCenter, Num nRadius);
  static Num Dist2Plane(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                        const CVector<4, Num> &vPlane, bool bHalfSpace);
  static Num Dist2Line(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax,
                       const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax);
  static Num Dist2Point(const CVector<3, Num> &vMin, const CVector<3, Num> &vMax, const CVector<3, Num> &vPoint);

  static void CalcFromPoints(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount,
                             CVector<3, Num> &vMin, CVector<3, Num> &vMax);
};

class COBB: public CShape3D {
  DEFRTTI(COBB, CShape3D, true)
public:
  CVector<3, Num> m_vCenter,
                  m_vExtent[3];

  COBB() { SetEmpty(); }
  COBB(const CVector<3, Num> &vCenter, const CVector<3, Num> &vExt0, const CVector<3, Num> &vExt1, const CVector<3, Num> &vExt2) { Init(vCenter, vExt0, vExt1, vExt2); }
  virtual ~COBB() {}

  void Init(const CVector<3, Num> &vCenter, const CVector<3, Num> &vExt0,
            const CVector<3, Num> &vExt1, const CVector<3, Num> &vExt2) { m_vCenter = vCenter; m_vExtent[0] = vExt0; m_vExtent[1] = vExt1; m_vExtent[2] = vExt2; }

  virtual void Init(CVector<3, Num> const *pPoints, int iStride, unsigned int uiCount);
  virtual void CopyFrom(CShape3D const *pSrc);
  virtual bool IsEmpty() const;
  virtual void SetEmpty();
  virtual void IncludePoint(CVector<3> const &v);
  virtual Num GetVolume() const;
  virtual CVector<3, Num> GetCenter() const { return m_vCenter; }
  virtual bool Dist(const CShape3D *pShape, Num &nDist) const;
  virtual CShape3D *GetTransformed(CXForm const &kXForm, CShape3D *pDstShape = 0) const;

  static void Transform(CVector<3, Num> const &vCenter, CVector<3, Num> const (&vExtent)[3], CXForm const &kXForm,
                        CVector<3, Num> &vXCenter, CVector<3, Num> (&vXExtent)[3]);

  // Convert OBB to AABB + transform such that OBB = transform(AABB)
  static void Untransform(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                          CVector<3, Num> &vMin, CVector<3, Num> &vMax, CXForm &kXForm);

  static CShape3D::Num CheckSidePoint(int iSide, int iPoint, CVector<4, Num> const (&vPlanes)[2],
                                      CVector<3, Num> const (&vPoints)[2][4], Num const (&nLen)[2][2],
                                      CVector<3, Num> const (&vBasis)[2][2]);
  static Num Side2SideDist(CVector<3, Num> const &vOrigin0, CVector<3, Num> const &vX0,
                           CVector<3, Num> const &vY0, CVector<4, Num> const &vPlane0,
                           CVector<3, Num> const &vOrigin1, CVector<3, Num> const &vX1,
                           CVector<3, Num> const &vY1, CVector<4, Num> const &vPlane1);
  static void GetSideData(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3], int iSide,
                          CVector<3, Num> &vOrigin, CVector<3, Num> &vX, CVector<3, Num> &vY, CVector<4, Num> &vPlane);


  static Num Dist2OBB(const CVector<3, Num> &vCenter0, const CVector<3, Num> (&vExtent0)[3],
                      const CVector<3, Num> &vCenter1, const CVector<3, Num> (&vExtent1)[3]);
  static Num Dist2AABB(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                       const CVector<3, Num> &vMin, const CVector<3, Num> &vMax);
  static Num Dist2Sphere(const CVector<3, Num> &vOBBCenter, const CVector<3, Num> (&vExtent)[3],
                         const CVector<3, Num> &vSphereCenter, Num nRadius);
  static Num Dist2Plane(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                        const CVector<4, Num> &vPlane, bool bHalfSpace);
  static Num Dist2Line(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3],
                       const CVector<3, Num> &vPoint0, const CVector<3, Num> &vPoint1, Num nLineMin, Num nLineMax);
  static Num Dist2Point(const CVector<3, Num> &vCenter, const CVector<3, Num> (&vExtent)[3], const CVector<3, Num> &vPoint);
};

#endif
