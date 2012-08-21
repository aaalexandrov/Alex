#ifndef __VECTOR_H
#define __VECTOR_H

#include "Debug.h"
#include "Var.h"
#include "Util.h"

template <int D = 3, class T = float>
class CVector {
public:
  static const unsigned int Dim = D;

  typedef T Num;

  Num m_Val[Dim];

  inline Num &Val(int iIndex)               { ASSERT(iIndex < Dim); return m_Val[iIndex]; }
  inline Num  Val(int iIndex) const         { ASSERT(iIndex < Dim); return m_Val[iIndex]; }

  inline Num &operator [](int iIndex)       { return Val(iIndex); }
  inline Num  operator [](int iIndex) const { return Val(iIndex); }

  inline Num &x()           { COMPILE_ASSERT(Dim > 0); return Val(0); }
  inline Num &y()           { COMPILE_ASSERT(Dim > 1); return Val(1); }
  inline Num &z()           { COMPILE_ASSERT(Dim > 2); return Val(2); }
  inline Num &w()           { COMPILE_ASSERT(Dim > 3); return Val(3); }

  inline Num x() const      { COMPILE_ASSERT(Dim > 0); return Val(0); }
  inline Num y() const      { COMPILE_ASSERT(Dim > 1); return Val(1); }
  inline Num z() const      { COMPILE_ASSERT(Dim > 2); return Val(2); }
  inline Num w() const      { COMPILE_ASSERT(Dim > 3); return Val(3); }

  inline int GetDim() const                       { return Dim; }

  inline bool operator ==(const CVector &v) const;
  inline bool operator !=(const CVector &v) const { return !operator==(v); }
  inline bool operator  <(const CVector &v) const;
  inline bool operator  >(const CVector &v) const { return v.operator <(*this);  }
  inline bool operator <=(const CVector &v) const { return !v.operator <(*this); }
  inline bool operator >=(const CVector &v) const { return !operator <(v);       }


  inline CVector &operator =(const CVector &v);
  inline CVector &operator =(Num n);

  inline CVector &operator +=(const CVector &v);
  inline CVector &operator -=(const CVector &v);
  inline CVector &operator *=(const CVector &v);
  inline CVector &operator /=(const CVector &v);

  inline CVector &operator +=(Num n);
  inline CVector &operator -=(Num n);
  inline CVector &operator *=(Num n);
  inline CVector &operator /=(Num n);

  inline CVector operator -() const;

  // Component-wise operations
  inline CVector operator +(const CVector &v) const;
  inline CVector operator -(const CVector &v) const;
  inline CVector operator *(const CVector &v) const { return Mul(*this, v);   }
  inline CVector operator /(const CVector &v) const { return Div(*this, v);   }
  inline CVector operator ^(const CVector &v) const { return Cross(*this, v); }
  inline Num     operator %(const CVector &v) const { return Dot(*this, v);   }

  inline CVector operator +(Num n) const;
  inline CVector operator -(Num n) const;
  inline CVector operator *(Num n) const;
  inline CVector operator /(Num n) const;

  static inline Num     Dot  (const CVector &a, const CVector &b);
  static inline CVector Cross(const CVector &a, const CVector &b);
  static inline CVector Mul  (const CVector &a, const CVector &b);
  static inline CVector Div  (const CVector &a, const CVector &b);

  inline Num     Dot  (const CVector &v) const { return Dot(*this, v); }
  inline CVector Cross(const CVector &v) const { return Cross(*this, v); }

  static inline bool IsEqual(Num m, Num n)     { return Util::IsEqual<Num>(m, n, Util::GetEqualDelta<Num>()); }
  inline Num Length() const;
  inline Num LengthSqr() const;
  inline CVector &SetLength(Num nLen);
  inline CVector &Normalize();
  inline CVector GetNormalized() const;
  inline bool IsParallel(const CVector &v, Num *pFactor = 0) const; // (*this) * (*pFactor) == v

  inline CVector Perpendicular() const;
  inline CVector Perpendicular2D() const;

  inline CVector& SetVal(Num nVal, int iIndex = -1);
  inline CVector& SetZero();
  inline CVector& SetUnit(int iIndex = -1);
  
  inline CVector &Set(Num nX);
  inline CVector &Set(Num nX, Num nY);
  inline CVector &Set(Num nX, Num nY, Num nZ);
  inline CVector &Set(Num nX, Num nY, Num nZ, Num nW);
  template <class V>
  inline CVector &Set(const V &v);

  static inline CVector GetVal(Num nVal, int iIndex = -1);
  static inline CVector GetZero();
  static inline CVector GetUnit(int iIndex = -1);

  static inline CVector Get(Num nX);
  static inline CVector Get(Num nX, Num nY);
  static inline CVector Get(Num nX, Num nY, Num nZ);
  static inline CVector Get(Num nX, Num nY, Num nZ, Num nW);
  template <class V>
  static inline CVector Get(const V &v);
};

template <class V>
V operator *(typename V::Num n, const V &v);

// Implementation -----------------------------------------------------------------

template <int D, class T>
bool CVector<D, T>::operator ==(const CVector &v) const
{
  for (int i = 0; i < Dim; i++)
    if (!IsEqual(Val(i), v.Val(i)))
      return false;
  return true;
}

template <int D, class T>
bool CVector<D, T>::operator <(const CVector &v) const
{
  for (int i = 0; i < Dim; i++)
    if (!(Val(i) < v.Val(i)))
      return false;
  return true;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator =(const CVector &v)
{
  for (int i = 0; i < Dim; i++)
    Val(i) = v.Val(i);
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator =(Num n)
{
  for (int i = 0; i < Dim; i++)
    Val(i) = n;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator +=(const CVector &v)
{
  for (int i = 0; i < Dim; i++)
    Val(i) += v.Val(i);
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator -=(const CVector &v)
{
  for (int i = 0; i < Dim; i++)
    Val(i) -= v.Val(i);
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator *=(const CVector &v)
{
  for (int i = 0; i < Dim; i++)
    Val(i) *= v.Val(i);
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator /=(const CVector &v)
{
  for (int i = 0; i < Dim; i++)
    Val(i) /= v.Val(i);
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator +=(Num n)
{
  for (int i = 0; i < Dim; i++)
    Val(i) += n;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator -=(Num n)
{
  for (int i = 0; i < Dim; i++)
    Val(i) -= n;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator *=(Num n)
{
  for (int i = 0; i < Dim; i++)
    Val(i) *= n;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::operator /=(Num n)
{
  for (int i = 0; i < Dim; i++)
    Val(i) /= n;
  return *this;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator -() const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = -Val(i);
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator +(const CVector &v) const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) + v.Val(i);
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator -(const CVector &v) const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) - v.Val(i);
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator +(Num n) const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) + n;
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator -(Num n) const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) - n;
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator *(Num n) const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) * n;
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::operator /(Num n) const
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) / n;
  return vRes;
}

template <int D, class T>
typename CVector<D, T>::Num CVector<D, T>::Dot(const CVector &a, const CVector &b)
{
  Num nRes = a.Val(0) * b.Val(0);
  for (int i = 1; i < Dim; i++)
    nRes += a.Val(i) * b.Val(i);
  return nRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Cross(const CVector &a, const CVector &b)
{
  COMPILE_ASSERT(Dim == 3);
  CVector vRes;
  vRes.x() = a.y() * b.z() - a.z() * b.y();
  vRes.y() = a.z() * b.x() - a.x() * b.z();
  vRes.z() = a.x() * b.y() - a.y() * b.x();
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Mul(const CVector &a, const CVector &b)
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = a.Val(i) * b.Val(i);
  return vRes;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Div(const CVector &a, const CVector &b)
{
  CVector vRes;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = a.Val(i) / b.Val(i);
  return vRes;
}

template <int D, class T>
typename CVector<D, T>::Num CVector<D, T>::Length() const
{
  return sqrt(LengthSqr());
}

template <int D, class T>
typename CVector<D, T>::Num CVector<D, T>::LengthSqr() const
{
  Num nLen = Val(0) * Val(0);
  for (int i = 1; i < Dim; i++)
    nLen += Val(i) * Val(i);
  return nLen;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::SetLength(Num nLen)
{
  Num nCurLen = Length();
  if (!IsEqual(nCurLen, 0))
    *this *= nLen / nCurLen;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::Normalize()
{
  Num nCurLen = Length();
  if (!IsEqual(nCurLen, 0))
    *this /= nCurLen;
  return *this;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::GetNormalized() const
{
  CVector vRes;
  Num nCurLen = Length();
  if (IsEqual(nCurLen, 0))
    return *this;
  for (int i = 0; i < Dim; i++)
    vRes.Val(i) = Val(i) / nCurLen;
  return vRes;
}

template <int D, class T>
bool CVector<D, T>::IsParallel(const CVector &v, Num *pFactor = 0) const
{
  int i;
  Num nFactor;
  if (!pFactor)
    pFactor = &nFactor;
  for (i = 0; i < Dim; i++) 
    if (!IsEqual(Val(i), 0))
      break;
  if (i >= Dim) {
    *pFactor = 0;
    return true;
  }
  *pFactor = v.Val(i) / Val(i);
  while (++i < Dim) 
    if (!IsEqual(Val(i) * (*pFactor), v.Val(i)))
      return false;
  return true;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Perpendicular() const
{
  CVector vPerp, v;
  int i;
  Num nMin, nDot;

  vPerp.SetUnit(0);
  nMin = abs(Dot(vPerp));
  for (i = 1; i < Dim; i++) {
    v.SetUnit(i);
    nDot = abs(Dot(v));
    if (nDot < nMin) {
      vPerp.SetUnit(i);
      nMin = nDot;
    }
  }
  nDot = Dot(vPerp) / LengthSqr();
  v = *this * nDot; 
  vPerp -= v;
  ASSERT(IsEqual(Dot(vPerp), 0));
  return vPerp;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Perpendicular2D() const
{
  COMPILE_ASSERT(Dim == 2);
  return Get(-y(), x());
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::SetVal(Num nVal, int iIndex)
{
  int i;
  if (iIndex < 0)
    for (i = 0; i < Dim; i++)
      Val(i) = nVal;
  else {
    i = 0;
    while (i < iIndex)
      Val(i++) = 0;
    Val(i++) = iIndex;
    while (i < Dim)
      Val(i++) = 0;
  }
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::SetZero()
{
  for (int i = 0; i < Dim; i++)
    Val(i) = 0;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::SetUnit(int iIndex)
{
  int i;
  if (iIndex < 0)
    for (i = 0; i < Dim; i++)
      Val(i) = 1;
  else
    for (i = 0; i < Dim; i++)
      Val(i) = (i == iIndex);
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::Set(Num nX)
{
  COMPILE_ASSERT(Dim == 1);
  x() = nX;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::Set(Num nX, Num nY)
{
  COMPILE_ASSERT(Dim == 2);
  x() = nX;
  y() = nY;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::Set(Num nX, Num nY, Num nZ)
{
  COMPILE_ASSERT(Dim == 3);
  x() = nX;
  y() = nY;
  z() = nZ;
  return *this;
}

template <int D, class T>
CVector<D, T> &CVector<D, T>::Set(Num nX, Num nY, Num nZ, Num nW)
{
  COMPILE_ASSERT(Dim == 4);
  x() = nX;
  y() = nY;
  z() = nZ;
  w() = nW;
  return *this;
}

template <int D, class T>
template <class V>
CVector<D, T> &CVector<D, T>::Set(const V &v)
{
  for (int i = 0; i < (int) Util::Min(Dim, V::Dim); i++)
    Val(i) = v.Val(i);
  return *this;
}

template <int D, class T>
CVector<D, T> CVector<D, T>::GetVal(Num nVal, int iIndex)
{
  CVector vRes;
  return vRes.SetVal(nVal, iIndex);
}

template <int D, class T>
CVector<D, T> CVector<D, T>::GetZero()
{
  CVector vRes;
  return vRes.SetZero();
}

template <int D, class T>
CVector<D, T> CVector<D, T>::GetUnit(int iIndex)
{
  CVector vRes;
  return vRes.SetUnit(iIndex);
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Get(Num nX)
{
  CVector vRes;
  return vRes.Set(nX);
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Get(Num nX, Num nY)
{
  CVector vRes;
  return vRes.Set(nX, nY);
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Get(Num nX, Num nY, Num nZ)
{
  CVector vRes;
  return vRes.Set(nX, nY, nZ);
}

template <int D, class T>
CVector<D, T> CVector<D, T>::Get(Num nX, Num nY, Num nZ, Num nW)
{
  CVector vRes;
  return vRes.Set(nX, nY, nZ, nW);
}

template <int D, class T>
template <class V>
CVector<D, T> CVector<D, T>::Get(const V &v)
{
  CVector vRes;
  vRes.Set(v);
  return vRes;
}

template <class V>
V operator *(typename V::Num n, const V &v)
{
  V vRes;
  for (int i = 0; i < V::Dim; i++)
    vRes.Val(i) = n * v.Val(i);
  return vRes;
}

// Vector Vars -------------------------------------------------------------

const CRTTI *GetVectorVarRTTI(int iDim, bool bVarRef = false);

template <int D, class T>
inline bool Set(int *dst, const CVector<D, T> *src) { ASSERT(0); return false; }
template <int D, class T>
inline bool Set(float *dst, const CVector<D, T> *src) { ASSERT(0); return false; }
template <int D, class T>
inline bool Set(CStrAny *dst, const CVector<D, T> *src) { ASSERT(0); return false; }
template <int D, class T>
inline bool Set(CVector<D, T> *dst, const int *src) { ASSERT(0); return false; }
template <int D, class T>
inline bool Set(CVector<D, T> *dst, const float *src) { ASSERT(0); return false; }
template <int D, class T>
inline bool Set(CVector<D, T> *dst, const CStrAny *src) { ASSERT(0); return false; }

template <int D, class T>
static inline bool SetValue(CVector<D, T> *val, const CBaseVar *vSrc)
{
  return SetValueBase<CVector<D, T> >(val, vSrc);
}

#endif