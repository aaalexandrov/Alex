#include "stdafx.h"
#include "Matrix.h"

#define _USE_MATH_DEFINES
#include <math.h>

// CVarMatrix -----------------------------------------------------------------

IMPRTTI_NOCREATE(CMatrixVar, CBaseVar)

CMatrixVar::CMatrixVar(int iRows, int iCols, UINT uiFlags, Num *pVal)
{
  m_iRows = iRows;
  m_iCols = iCols;
  m_uiFlags = uiFlags;
  if (m_uiFlags & MVF_OWNVALUES) {
    m_pVal = new Num[iRows * iCols];
    if (pVal)
      memcpy(m_pVal, pVal, iRows * iCols * sizeof(Num));
  } else
    m_pVal = pVal;
}

CMatrixVar::Num &CMatrixVar::At(int iRow, int iCol)
{
  iRow = Util::Bound(iRow, 0, m_iRows - 1);
  iCol = Util::Bound(iCol, 0, m_iCols - 1);
  if (m_uiFlags & MVF_TRANSPOSED) 
    return m_pVal[iCol * m_iRows + iRow];
  else
    return m_pVal[iRow * m_iCols + iCol];
}

const CMatrixVar::Num &CMatrixVar::At(int iRow, int iCol) const
{
  iRow = Util::Bound(iRow, 0, m_iRows - 1);
  iCol = Util::Bound(iCol, 0, m_iCols - 1);
  if (m_uiFlags & MVF_TRANSPOSED) 
    return m_pVal[iCol * m_iRows + iRow];
  else
    return m_pVal[iRow * m_iCols + iCol];
}

bool CMatrixVar::SetVar(const CBaseVar &vSrc)
{
  const CMatrixVar *pMV = Cast<CMatrixVar>(&vSrc);
  if (!pMV)
    return false;
  for (int r = 0; r < Util::Min(m_iRows, pMV->m_iRows); r++)
    for (int c = 0; c < Util::Min(m_iCols, pMV->m_iCols); c++)
      At(r, c) = pMV->At(r, c);
  return m_iRows >= pMV->m_iRows && m_iCols >= pMV->m_iCols;
}

// Matrix Vars ----------------------------------------------------------------

#define IMP_MAT_VAR_RTTI(ROWS, COLS) \
  IMP_VAR_RTTI(ID(CONCAT(CMatrix<ROWS, COLS>))) \
  IMP_VAR_RTTI(ID(CONCAT(CMatT<CMatrix<ROWS, COLS>>)))

IMP_MAT_VAR_RTTI(2, 2)
IMP_MAT_VAR_RTTI(2, 3)
IMP_MAT_VAR_RTTI(2, 4)
IMP_MAT_VAR_RTTI(3, 2)
IMP_MAT_VAR_RTTI(3, 3)
IMP_MAT_VAR_RTTI(3, 4)
IMP_MAT_VAR_RTTI(4, 2)
IMP_MAT_VAR_RTTI(4, 3)
IMP_MAT_VAR_RTTI(4, 4)

const CRTTI *GetMatrixVarRTTI(int iRows, int iCols, bool bVarRef)
{
  static CRTTI *pMatrixRTTI[2][4][4] = 
    { { { 0, 0,                             0,                             0                             },
        { 0, &CVar<CMatrix<2, 2> >::s_RTTI, &CVar<CMatrix<2, 3> >::s_RTTI, &CVar<CMatrix<2, 4> >::s_RTTI },
        { 0, &CVar<CMatrix<3, 2> >::s_RTTI, &CVar<CMatrix<3, 3> >::s_RTTI, &CVar<CMatrix<3, 4> >::s_RTTI },
        { 0, &CVar<CMatrix<4, 2> >::s_RTTI, &CVar<CMatrix<4, 3> >::s_RTTI, &CVar<CMatrix<4, 4> >::s_RTTI } },

      { { 0, 0,                                0,                                0                                },
        { 0, &CVarRef<CMatrix<2, 2> >::s_RTTI, &CVarRef<CMatrix<2, 3> >::s_RTTI, &CVarRef<CMatrix<2, 4> >::s_RTTI },
        { 0, &CVarRef<CMatrix<3, 2> >::s_RTTI, &CVarRef<CMatrix<3, 3> >::s_RTTI, &CVarRef<CMatrix<3, 4> >::s_RTTI },
        { 0, &CVarRef<CMatrix<4, 2> >::s_RTTI, &CVarRef<CMatrix<4, 3> >::s_RTTI, &CVarRef<CMatrix<4, 4> >::s_RTTI } } };

  class CMatRTTIInit {
  public:
    CMatRTTIInit() {
      for (int ref = 0; ref < 2; ref++)
        for (int r = 0; r < 4; r++)
          for (int c = 0; c < 4; c++)
            if (pMatrixRTTI[ref][r][c])
              pMatrixRTTI[ref][r][c]->m_uiClassData = (r << 8) | c;
    }
  };

  static CMatRTTIInit kMatRTTIInit;

  if (iRows < 1 || iRows > ARRSIZE(pMatrixRTTI[0]) || iCols < 1 || iCols > ARRSIZE(pMatrixRTTI[0][0]))
    return 0;
  return pMatrixRTTI[!!bVarRef][iRows - 1][iCols - 1];
}

const CRTTI *GetMatTVarRTTI(int iRows, int iCols, bool bVarRef)
{
  static CRTTI *pMatTRTTI[2][4][4] = 
    { { { 0, 0,                                     0,                                     0                                     },
        { 0, &CVar<CMatT<CMatrix<2, 2> > >::s_RTTI, &CVar<CMatT<CMatrix<2, 3> > >::s_RTTI, &CVar<CMatT<CMatrix<2, 4> > >::s_RTTI },
        { 0, &CVar<CMatT<CMatrix<3, 2> > >::s_RTTI, &CVar<CMatT<CMatrix<3, 3> > >::s_RTTI, &CVar<CMatT<CMatrix<3, 4> > >::s_RTTI },
        { 0, &CVar<CMatT<CMatrix<4, 2> > >::s_RTTI, &CVar<CMatT<CMatrix<4, 3> > >::s_RTTI, &CVar<CMatT<CMatrix<4, 4> > >::s_RTTI } },

      { { 0, 0,                                        0,                                        0                                        },
        { 0, &CVarRef<CMatT<CMatrix<2, 2> > >::s_RTTI, &CVarRef<CMatT<CMatrix<2, 3> > >::s_RTTI, &CVarRef<CMatT<CMatrix<2, 4> > >::s_RTTI },
        { 0, &CVarRef<CMatT<CMatrix<3, 2> > >::s_RTTI, &CVarRef<CMatT<CMatrix<3, 3> > >::s_RTTI, &CVarRef<CMatT<CMatrix<3, 4> > >::s_RTTI },
        { 0, &CVarRef<CMatT<CMatrix<4, 2> > >::s_RTTI, &CVarRef<CMatT<CMatrix<4, 3> > >::s_RTTI, &CVarRef<CMatT<CMatrix<4, 4> > >::s_RTTI } } };

  class CMatTRTTIInit {
  public:
    CMatTRTTIInit() {
      for (int ref = 0; ref < 2; ref++)
        for (int r = 0; r < 4; r++)
          for (int c = 0; c < 4; c++)
            if (pMatTRTTI[ref][r][c])
              pMatTRTTI[ref][r][c]->m_uiClassData = (r << 8) | c;
    }
  };

  static CMatTRTTIInit kMatTRTTIInit;

  if (iRows < 1 || iRows > ARRSIZE(pMatTRTTI[0]) || iCols < 1 || iCols > ARRSIZE(pMatTRTTI[0][0]))
    return 0;
  return pMatTRTTI[!!bVarRef][iRows - 1][iCols - 1];
}

struct TMatrixTest {
  TMatrixTest() {
    CMatrix<4, 3> m43;
    CMatrix<3, 4> m34;
    CMatrix<3, 3> m33, m33_1;
    CMatrix<2, 4> m24;
    CMatrix<2, 3> m23;

    m33.SetDiagonal();
    m33_1 = m33 * 2;

    CVector<> v = {{1, 2, 3}}, v1, v2;
    v1 = m33_1 * v;
    v2 = v * m33_1;

    m33 = m33_1.GetTransposed().GetInverse();
    float f = m33.Determinant();
    float f1 = m33_1.Determinant();

    CVector<4> v4;
    m34.SetDiagonal();
    m43 = m34.GetTransposed();
    v4 = v2 * m34;
    m24.Set(1);
    m23.SetMul(m24, m43);
    m33.Transpose();
    CMatrix<> m44;
    m44.SetTranslation(CVector<3>::Get(1, 2, 3));
    m43.SetScale(CVector<3>::Get(0.5, 0.5, 0.5));
    m34.SetRotation(0, PI / 2);

    int r, c, r1;
    for (r = 0; r < 4; r++)
      for (c = 0; c < 4; c++)
        m44(r, c) = (r <= c) * 5.0f;

    m44.Orthonormalize();

    for (r = 0; r < 4; r++) {
      ASSERT(abs(m44.GetRow(r).Length() - 1) < 0.001);
      for (r1 = r + 1; r1 < 4; r1++) {
        float fDot = m44.GetRow(r) % m44.GetRow(r1);
        ASSERT(abs(fDot) < 0.001);
      }
    }

    CVar<CMatrix<3, 4> > v34;
    for (r = 0; r < 3; r++)
      for (c = 0; c < 4; c++)
        v34.Val()(r, c) = (float) r * 10 + c;

    CVar<CMatT<CMatrix<3, 4> > > v43;

    v43.SetVar(v34);

    CMatrix<4, 4> mt44;

    CVarRef<CMatrix<4, 4> > v44(m44);
    for (r = 0; r < 4; r++)
      for (c = 0; c < 4; c++)
        v44.Val()(r, c) = (float) r * 10 + c;

    CVarRef<CMatT<CMatrix<4, 4> > > vt44;
    vt44.SetRef(&mt44);

    vt44.SetVar(v44);

  }
} /*g_MatrixTest*/;