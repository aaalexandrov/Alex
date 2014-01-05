#ifndef __RECT_H
#define __RECT_H

#include "Vector.h"
#include "Util.h"

template <class T = float>
class CRect {
public:
  typedef T Num;
  typedef CVector<2, Num> Vec;

  Vec m_vMin, m_vMax;

  inline CRect() {}
  inline CRect(T x1, T y1, T x2, T y2)           { Set(x1, y1, x2, y2); }
  inline CRect(const Vec &vMin, const Vec &vMax) { m_vMin = vMin; m_vMax = vMax; }
  inline CRect(const CRect &rc)                  { operator =(rc); }

  inline bool operator ==(const CRect &rc)       { return m_vMin == rc.m_vMin && m_vMax == rc.m_vMax; }
  inline bool operator !=(const CRect &rc)       { return m_vMin != rc.m_vMin || m_vMax != rc.m_vMax; }
  inline CRect &operator =(const CRect &rc);
  inline CRect &Set(T x1, T y1, T x2, T y2);
  inline CRect &SetEmpty();

  inline bool IsEmpty() const                    { return m_vMin > m_vMax; }

  inline Vec  GetSize() const                    { return m_vMax - m_vMin; }
  inline Num  GetWidth() const                   { return m_vMax.x() - m_vMin.x(); }
  inline Num  GetHeight() const                  { return m_vMax.y() - m_vMin.y(); }

  inline Vec  GetPoint(Num nX, Num nY)           { return m_vMin + Vec::Get(nX, nY) * GetSize(); }

  inline bool Contains(const Vec &v) const       { return m_vMin <= v && v <= m_vMax; }
  inline Vec &MoveInside(Vec &v) const;
  inline CRect &AddPoint(Vec &v);

  inline bool Intersects(const CRect &rc) const  { return !IsEmpty() && m_vMin <= rc.m_vMax && rc.m_vMin <= m_vMax; }
  inline bool Contains(const CRect &rc) const    { return !IsEmpty() && m_vMin <= rc.m_vMin && rc.m_vMax <= m_vMax; }
  inline CRect GetIntersection(const CRect &rc) const;
  inline CRect &MoveInside(CRect &rc) const;
  inline CRect &AddRect(CRect const &rc);
};

// Implementation ------------------------------------------------------------------------

template <class T>
CRect<T> &CRect<T>::operator =(const CRect &rc)
{
  m_vMin = rc.m_vMin;
  m_vMax = rc.m_vMax;
  return *this;
}

template <class T>
CRect<T> &CRect<T>::Set(T x1, T y1, T x2, T y2)
{ 
  m_vMin.Set(x1, y1); 
  m_vMax.Set(x2, y2); 
  return *this; 
}

template <class T>
CRect<T> &CRect<T>::SetEmpty()
{
  m_vMin = 1;
  m_vMax = 0;
  return *this;
}

template <class T>
typename CRect<T>::Vec &CRect<T>::MoveInside(Vec &v) const
{
  v.x() = Util::Bound(v.x(), m_vMin.x(), m_vMax.x());
  v.y() = Util::Bound(v.y(), m_vMin.y(), m_vMax.y());
  return v;
}

template <class T>
CRect<T> &CRect<T>::AddPoint(Vec &v)
{
  if (IsEmpty()) {
    m_vMin = m_vMax = v;
    return *this;
  }
  m_vMin.x() = Util::Min(m_vMin.x(), v.x());
  m_vMin.y() = Util::Min(m_vMin.y(), v.y());
  m_vMax.x() = Util::Max(m_vMax.x(), v.x());
  m_vMax.y() = Util::Max(m_vMax.y(), v.y());
  return *this;
}

template <class T>
CRect<T> CRect<T>::GetIntersection(const CRect &rc) const
{
  CRect rcRes;
  rcRes.m_vMin.x() = Util::Max(m_vMin.x(), rc.m_vMin.x());
  rcRes.m_vMin.y() = Util::Max(m_vMin.y(), rc.m_vMin.y());
  rcRes.m_vMax.x() = Util::Min(m_vMax.x(), rc.m_vMax.x());
  rcRes.m_vMax.y() = Util::Min(m_vMax.y(), rc.m_vMax.y());
  return rcRes;
}

template <class T>
CRect<T> &CRect<T>::MoveInside(CRect &rc) const
{
  rc.m_vMin.x() = Util::Max(m_vMin.x(), rc.m_vMin.x());
  rc.m_vMin.y() = Util::Max(m_vMin.y(), rc.m_vMin.y());
  rc.m_vMax.x() = Util::Min(m_vMax.x(), rc.m_vMax.x());
  rc.m_vMax.y() = Util::Min(m_vMax.y(), rc.m_vMax.y());
  return rc;
}

template <class T>
CRect<T> &CRect<T>::AddRect(CRect const &rc)
{
  if (IsEmpty())
    *this = rc;
  if (rc.IsEmpty())
    return *this;
  m_vMin.x() = Util::Min(m_vMin.x(), rc.m_vMin.x());
  m_vMin.y() = Util::Min(m_vMin.y(), rc.m_vMin.y());
  m_vMax.x() = Util::Max(m_vMax.x(), rc.m_vMax.x());
  m_vMax.y() = Util::Max(m_vMax.y(), rc.m_vMax.y());
  return *this;
}

#endif