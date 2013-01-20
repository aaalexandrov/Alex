#ifndef __MATRIX_H
#define __MATRIX_H

#include "Debug.h"
#include "Util.h"
#include "Var.h"
#include "Vector.h"

#define MATRIX_CHECK_ARITHMETIC false

template <int R = 4, int C = 4, class T = float>
class CMatrix {
public:
  static const unsigned int Rows = R;
  static const unsigned int Cols = C;

  typedef T Num;
  typedef CVector<Cols, Num> VecRow;
  typedef CVector<Rows, Num> VecCol;
  typedef CMatrix<Rows, Cols, Num> Mat;
  typedef CMatrix<Cols, Rows, Num> MatT;

  Num m_Val[Rows][Cols];

  inline       Num &Val(int iRow, int iCol)       { return m_Val[iRow][iCol]; }
  inline const Num &Val(int iRow, int iCol) const { return m_Val[iRow][iCol]; }

  inline       Num &operator ()(int iRow, int iCol)       { return Val(iRow, iCol); }
  inline const Num &operator ()(int iRow, int iCol) const { return Val(iRow, iCol); }

  inline int GetRows() const               { return Rows; }
  inline int GetCols() const               { return Cols; }

  static inline bool IsEqual(Num m, Num n, Num nEps = 0.0005) { return abs(m - n) <= nEps; }

  inline bool     operator ==(const CMatrix &m) const;

  inline CMatrix &operator =(const CMatrix &m);
  inline CMatrix &Set(Num n = 0);
  inline CMatrix &SetZero()                { return Set(); }
  inline CMatrix &SetDiagonal(Num n = 1);

  inline VecRow  &GetRow(int iRow);
  inline VecRow   GetRow(int iRow) const;
  inline VecCol   GetCol(int iCol) const;
  inline CMatrix &SetRow(int iRow, const VecRow &v);
  inline CMatrix &SetCol(int iCol, const VecCol &v);

  inline CMatrix operator -() const;

  // Component-wise operations
  inline CMatrix operator +(const CMatrix &m) const;
  inline CMatrix operator -(const CMatrix &m) const;
  inline CMatrix operator *(const CMatrix &m) const;
  inline CMatrix operator /(const CMatrix &m) const;

  inline CMatrix operator +(Num n) const;
  inline CMatrix operator -(Num n) const;
  inline CMatrix operator *(Num n) const;
  inline CMatrix operator /(Num n) const;

  inline VecCol   operator *(const VecRow &v) const;
  template <int K>
  inline CMatrix &SetMul(const CMatrix<R, K, T> &m1, const CMatrix<K, C, T> &m2);

  inline CMatrix &Transpose();
  inline MatT     GetTransposed() const;
  inline MatT    &GetTransposed(MatT &mResult) const;

  inline Num     Determinant() const;
  inline CMatrix GetInverse() const;
  inline CMatrix &GetInverse(CMatrix &mResult) const;

  template <int K>
  inline bool MakeTriangular(bool bUnitDiagonal = false, CMatrix<R, K, T> *pMirror = 0);
  template <int K>
  inline bool MakeZeroAboveDiagonal(CMatrix<R, K, T> *pMirror = 0);
  inline void SwapRows(int iRow1, int iRow2);
  inline void AddRows(Num nCoef, int iRowSrc, int iRowDst);
  inline void MulRow(Num nCoef, int iRow);

  inline CMatrix &SetRotation    (int iAxis, Num nRadians);
  inline CMatrix &SetScale       (const CVector<Rows - 1, Num> &vScale);
  inline CMatrix &SetTranslation (const CVector<Rows - 1, Num> &vTrans);
  inline CMatrix &SetProjection  (Num nWHRatio, Num nHorizontalFOV, Num nNear, Num nFar);
  inline CMatrix &SetOrthographic(Num nLeft, Num nTop, Num nRight, Num nBottom, Num nNear, Num nFar);

  inline CMatrix &Orthogonalize();
  inline CMatrix &Normalize();
  inline CMatrix &Orthonormalize() { return Orthogonalize().Normalize(); }
};

template <class M>
inline M operator *(typename M::Num n, const typename M::Mat &m);

template <class M>
inline typename M::VecRow operator *(const typename M::VecCol &v, const M &m);

// Implementation -----------------------------------------------------------------

template <int R, int C, class T>
bool CMatrix<R, C, T>::operator ==(const CMatrix &m) const
{
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      if (!(IsEqual(Val(r, c), m.Val(r, c))))
        return false;
  return true;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::operator =(const CMatrix &m)
{
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      Val(r, c) = m.Val(r, c);
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::Set(Num n)
{
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      Val(r, c) = n;
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetDiagonal(Num n)
{
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      Val(r, c) = (r == c) ? n : 0;
  return *this;
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::VecRow &CMatrix<R, C, T>::GetRow(int iRow)
{
  return *(VecRow *) &Val(iRow, 0);
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::VecRow CMatrix<R, C, T>::GetRow(int iRow) const
{
  VecRow vRes;
  for (int c = 0; c < Cols; c++)
    vRes[c] = Val(iRow, c);
  return vRes;
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::VecCol CMatrix<R, C, T>::GetCol(int iCol) const
{
  VecCol vRes;
  for (int r = 0; r < Rows; r++)
    vRes[r] = Val(r, iCol);
  return vRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetRow(int iRow, const VecRow &v)
{
  for (int c = 0; c < Cols; c++)
    Val(iRow, c) = v[c];
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetCol(int iCol, const VecCol &v)
{
  for (int r = 0; r < Rows; r++)
    Val(r, iCol) = v[r];
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator -() const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = -Val(r, c);
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator +(const CMatrix &m) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) + m.Val(r, c);
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator -(const CMatrix &m) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) - m.Val(r, c);
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator *(const CMatrix &m) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) * m.Val(r, c);
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator /(const CMatrix &m) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) / m.Val(r, c);
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator +(Num n) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) + n;
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator -(Num n) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) - n;
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator *(Num n) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) * n;
  return mRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::operator /(Num n) const
{
  CMatrix mRes;
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mRes.Val(r, c) = Val(r, c) / n;
  return mRes;
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::VecCol CMatrix<R, C, T>::operator *(const VecRow &v) const
{
  VecCol vRes;
  for (int r = 0; r < Rows; r++) {
    vRes[r] = Val(r, 0) * v[0];
    for (int c = 1; c < Cols; c++)
      vRes[r] += Val(r, c) * v[c];
  }
  return vRes;
}

template <int R, int C, class T>
template <int K>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetMul(const CMatrix<R, K, T> &m1, const CMatrix<K, C, T> &m2)
{
  for (int r = 0; r < m1.GetRows(); r++)
    for (int c = 0; c < m2.GetCols(); c++) {
      Val(r, c) = m1.Val(r, 0) * m2.Val(0, c);
      for (int i = 1; i < m1.GetCols(); i++)
        Val(r, c) += m1.Val(r, i) * m2.Val(i, c);
    }
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::Transpose()
{
  COMPILE_ASSERT(Rows == Cols);
  for (int r = 1; r < Rows; r++)
    for (int c = 0; c < r; c++)
      Util::Swap(Val(r, c), Val(c, r));
  return *this;
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::MatT CMatrix<R, C, T>::GetTransposed() const
{
  MatT mRes;
  return GetTransposed(mRes);
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::MatT &CMatrix<R, C, T>::GetTransposed(typename CMatrix<R, C, T>::MatT &mResult) const
{
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      mResult.Val(c, r) = Val(r, c);
  return mResult;
}

template <int R, int C, class T>
typename CMatrix<R, C, T>::Num CMatrix<R, C, T>::Determinant() const
{
  COMPILE_ASSERT(Rows == Cols);
  int r;
  CMatrix m = *this;
  if (!m.MakeTriangular<C>())
    return 0;
  Num nRes = m.Val(0, 0);
  for (r = 1; r < Rows; r++)
    nRes *= m.Val(r, r);
  return nRes;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::GetInverse(CMatrix &mResult) const
{
  COMPILE_ASSERT(Rows == Cols);
  CMatrix m;
  m = *this;
  mResult.SetDiagonal();
  if (!m.MakeTriangular(true, &mResult))
    mResult.SetZero();
  else
    m.MakeZeroAboveDiagonal(&mResult);
  return mResult;
}

template <int R, int C, class T>
CMatrix<R, C, T> CMatrix<R, C, T>::GetInverse() const
{
  CMatrix mInv;
  return GetInverse(mInv);
}

template <int R, int C, class T>
template <int K>
bool CMatrix<R, C, T>::MakeTriangular(bool bUnitDiagonal, CMatrix<R, K, T> *pMirror)
{
//  COMPILE_ASSERT(Rows == Cols);
  int r, i;
  Num nCoef;
  for (r = 0; r < Rows; r++) {
    for (i = r; i < Rows; i++)
      if (!IsEqual(Val(i, r), 0))
        break;
    if (i != r) {
      if (i >= Rows)
        return false;
      SwapRows(i, r);
      if (pMirror)
        pMirror->SwapRows(i, r);
    }
    ASSERT(!MATRIX_CHECK_ARITHMETIC || !IsEqual(Val(r, r), 0));
    if (bUnitDiagonal && !IsEqual(Val(r, r), 1)) {
      nCoef = 1 / Val(r, r);
      MulRow(nCoef, r);
      ASSERT(IsEqual(Val(r, r), 1));
      if (pMirror)
        pMirror->MulRow(nCoef, r);
    }
    for (i = r + 1; i < Rows; i++)
      if (!IsEqual(Val(i, r), 0)) {
        nCoef = -Val(i, r) / Val(r, r);
        AddRows(nCoef, r, i);
        ASSERT(!MATRIX_CHECK_ARITHMETIC || IsEqual(Val(i, r), 0));
        if (pMirror)
          pMirror->AddRows(nCoef, r, i);
      }
  }
  return true;
}

template <int R, int C, class T>
template <int K>
bool CMatrix<R, C, T>::MakeZeroAboveDiagonal(CMatrix<R, K, T> *pMirror)
{
  int r, i;
  Num nCoef;
  for (r = Rows - 1; r >= 0; r--) {
    ASSERT(!MATRIX_CHECK_ARITHMETIC || !IsEqual(Val(r, r), 0));
    for (i = 0; i < r; i++)
      if (!IsEqual(Val(i, r), 0)) {
        nCoef = -Val(i, r) / Val(r, r);
        AddRows(nCoef, r, i);
        ASSERT(!MATRIX_CHECK_ARITHMETIC || IsEqual(Val(i, r), 0));
        if (pMirror)
          pMirror->AddRows(nCoef, r, i);
      }
  }
  return true;
}

template <int R, int C, class T>
void CMatrix<R, C, T>::SwapRows(int iRow1, int iRow2)
{
  for (int c = 0; c < Cols; c++)
    Util::Swap(Val(iRow1, c), Val(iRow2, c));
}

template <int R, int C, class T>
void CMatrix<R, C, T>::AddRows(Num nCoef, int iRowSrc, int iRowDst)
{
  for (int c = 0; c < Cols; c++)
    Val(iRowDst, c) += nCoef * Val(iRowSrc, c);
}

template <int R, int C, class T>
void CMatrix<R, C, T>::MulRow(Num nCoef, int iRow)
{
  for (int c = 0; c < Cols; c++)
    Val(iRow, c) *= nCoef;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetRotation(int iAxis, Num nRadians)
{
  int i1, i2;
  if (iAxis >= 3)
    return SetDiagonal();
  i1 = (iAxis + 1) % 3;
  i2 = (iAxis + 2) % 3;
  Val(iAxis, iAxis) = 1;
  Val(iAxis, i1) = 0;
  Val(iAxis, i2) = 0;
  Val(i1, iAxis) = 0;
  Val(i1, i1) = cos(nRadians);
  Val(i1, i2) = sin(nRadians);
  Val(i2, iAxis) = 0;
  Val(i2, i1) = -Val(i1, i2);
  Val(i2, i2) = Val(i1, i1);
  for (int r = 0; r < Rows; r++)
    for (int c = (r < 3) ? 3 : 0; c < Cols; c++)
      Val(r, c) = (r == c);
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetScale(const CVector<Rows - 1, Num> &vScale)
{
  for (int r = 0; r < Rows; r++)
    for (int c = 0; c < Cols; c++)
      if (r == c)
        Val(r, c) = (r < Rows - 1) ? vScale[r] : 1;
      else
        Val(r, c) = 0;
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetTranslation(const CVector<Rows - 1, Num> &vTrans)
{
  COMPILE_ASSERT(Cols >= Rows - 1);
  int r, c;
  for (r = 0; r < Rows - 1; r++)
    for (c = 0; c < Cols; c++)
      Val(r, c) = (r == c);
  for (c = 0; c < Rows - 1; c++)
    Val(Rows - 1, c) = vTrans[c];
  for (c = Rows - 1; c < Cols; c++)
    Val(Rows - 1, c) = (c == Rows - 1);
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetProjection(Num nWHRatio, Num nHorizontalFOV, Num nNear, Num nFar)
{
  COMPILE_ASSERT(Rows == 4 && Cols == 4);
  Val(0, 0) = 1 / tan(nHorizontalFOV / 2);
  Val(0, 1) = Val(0, 2) = Val(0, 3) = 0;
  Val(1, 0) = 0;
  Val(1, 1) = Val(0, 0) * nWHRatio;
  Val(1, 2) = Val(1, 3) = 0;
  Val(2, 0) = Val(2, 1) = 0;
  Val(2, 2) = nFar / (nFar - nNear);
  Val(2, 3) = 1;
  Val(3, 0) = Val(3, 1) = 0;
  Val(3, 2) = - Val(2, 2) * nNear;
  Val(3, 3) = 0;
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::SetOrthographic(Num nLeft, Num nTop, Num nRight, Num nBottom, Num nNear, Num nFar)
{
  COMPILE_ASSERT(Rows == 4 && Cols == 4);
  Num nWidth, nHeight;
  nWidth = nRight - nLeft;
  nHeight = nBottom - nTop;
  Val(0, 0) = 2 / nWidth;
  Val(0, 1) = Val(0, 2) = Val(0, 3) = 0;
  Val(1, 0) = 0;
  Val(1, 1) = 2 / nHeight;
  Val(1, 2) = Val(1, 3) = 0;
  Val(2, 0) = Val(2, 1) = 0;
  Val(2, 2) = 1 / (nFar - nNear);
  Val(2, 3) = 0;
  Val(3, 0) = -(nLeft + nRight) / nWidth;
  Val(3, 1) = -(nTop + nBottom) / nHeight;
  Val(3, 2) = - Val(2, 2) * nNear;
  Val(3, 3) = 1;
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::Orthogonalize()
{
  COMPILE_ASSERT(Cols >= Rows);
  for (int r = 1; r < Rows; r++) {
    VecRow &v = GetRow(r);
    VecRow vExtra;
    vExtra.SetZero();
    for (int r1 = 0; r1 < r; r1++) {
      VecRow &vPrev = GetRow(r1);
      vExtra += ((v % vPrev) / (vPrev % vPrev)) * vPrev;
    }
    v -= vExtra;
  }
  return *this;
}

template <int R, int C, class T>
CMatrix<R, C, T> &CMatrix<R, C, T>::Normalize()
{
  for (int r = 0; r < Rows; r++)
    GetRow(r).Normalize();
  return *this;
}

template <class M>
M operator *(typename M::Num n, const typename M::Mat &m)
{
  M mRes;
  for (int r = 0; r < M::Rows; r++)
    for (int c = 0; c < M::Cols; c++)
      mRes.Val(r, c) = n * m.Val(r, c);
  return mRes;
}

template <class M>
typename M::VecRow operator *(const typename M::VecCol &v, const M &m)
{
  typename M::VecRow vRes;
  for (int c = 0; c < M::Cols; c++) {
    vRes[c] = v[0] * m.Val(0, c);
    for (int r = 1; r < M::Rows; r++)
      vRes[c] += v[r] * m.Val(r, c);
  }
  return vRes;
}

// CVarMatrix -----------------------------------------------------------------

class CMatrixVar: public CBaseVar {
  DEFRTTI_NOCREATE
public:
  typedef float Num;

  enum EFlags {
    MVF_OWNVALUES = 1,
    MVF_TRANSPOSED = 2,
  };
public:
  int m_iRows, m_iCols;
  Num *m_pVal;
  UINT m_uiFlags;

  CMatrixVar(int iRows, int iCols, UINT uiFlags = MVF_OWNVALUES, Num *pVal = 0);
  virtual ~CMatrixVar() { if (m_uiFlags & MVF_OWNVALUES) delete [] m_pVal; }

  virtual void *GetPtr()  const { return m_pVal; }
  virtual int   GetSize() const { return m_iRows * m_iCols * sizeof(Num); }

  virtual bool GetStr(CStrAny &s) const  { ASSERT(0); return false; }
  virtual bool SetStr(CStrAny const &s)  { ASSERT(0); return false; }

  virtual bool GetInt(int &i) const      { ASSERT(0); return false; }
  virtual bool SetInt(int i)             { ASSERT(0); return false; }

  virtual bool GetFloat(float &f) const  { ASSERT(0); return false; }
  virtual bool SetFloat(float f)         { ASSERT(0); return false; }

  template <int R, int C>
  bool GetMatrix(CMatrix<R, C, Num> &m) const;
  template <int R, int C>
  bool SetMatrix(const CMatrix<R, C, Num> &m);

  virtual Num &At(int iRow, int iCol);
  virtual const Num &At(int iRow, int iCol) const;

  virtual bool SetVar(const CBaseVar &vSrc);

  virtual void *GetRef() const     { return m_pVal; }
  virtual bool  SetRef(void *pPtr) { if (m_uiFlags & MVF_OWNVALUES) return false; m_pVal = (Num *) pPtr; return true; }

  virtual bool ValueHasRTTI() const { return false; }

  virtual CBaseVar *Clone() const { return new CMatrixVar(m_iRows, m_iCols, m_uiFlags, m_pVal); }
};

template <int R, int C>
bool CMatrixVar::GetMatrix(CMatrix<R, C, Num> &m) const
{
  if (!m_pVal)
    return false;
  int r, c;
  for (r = 0; r < Util::Min(R, m_iRows); r++)
    for (c = 0; c < Util::Min(C, m_iCols); c++)
      m(r, c) = At(r, c);
  return R == m_iRows && C == m_iCols;
}

template <int R, int C>
bool CMatrixVar::SetMatrix(const CMatrix<R, C, Num> &m)
{
  if (!m_pVal)
    return false;
  int r, c;
  for (r = 0; r < Util::Min(R, m_iRows); r++)
    for (c = 0; c < Util::Min(C, m_iCols); c++)
      At(r, c) = m(r, c);
  return R == m_iRows && C == m_iCols;
}

// Matrix Vars ----------------------------------------------------------------

const CRTTI *GetMatrixVarRTTI(int iRows, int iCols, bool bVarRef = false);

template <int R, int C, class T>
inline bool Set(int *dst, const CMatrix<R, C, T> *src) { ASSERT(0); return false; }
template <int R, int C, class T>
inline bool Set(float *dst, const CMatrix<R, C, T> *src) { ASSERT(0); return false; }
template <int R, int C, class T>
inline bool Set(CStrAny *dst, const CMatrix<R, C, T> *src) { ASSERT(0); return false; }
template <int R, int C, class T>
inline bool Set(CMatrix<R, C, T> *dst, const int *src) { ASSERT(0); return false; }
template <int R, int C, class T>
inline bool Set(CMatrix<R, C, T> *dst, const float *src) { ASSERT(0); return false; }
template <int R, int C, class T>
inline bool Set(CMatrix<R, C, T> *dst, const CStrAny *src) { ASSERT(0); return false; }

template <int R, int C, class T>
static inline bool SetValue(CMatrix<R, C, T> *val, const CBaseVar *vSrc)
{
  return SetValueBase(val, vSrc);
}

// Transposed matrix var helper class ---------------------------------------

const CRTTI *GetMatTVarRTTI(int iRows, int iCols, bool bVarRef = false);

template <class M>
class CMatT: public M::MatT {
};

template <class M>
static inline bool SetValue(CMatT<M> *val, const CBaseVar *vSrc)
{
  const CVarValueBase<M> *pV = Cast<CVarValueBase<M> >(vSrc);
  if (pV) {
    pV->GetValue().GetTransposed(* (typename M::MatT *) val);
    return true;
  }
  return SetValueBase(val, vSrc);
}

#endif
