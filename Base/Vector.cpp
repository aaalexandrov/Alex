#include "stdafx.h"
#include "Vector.h"

CVarRTTIRegisterer<CVector<2> > g_RegVarVector2;
CVarRTTIRegisterer<CVector<3> > g_RegVarVector3;
CVarRTTIRegisterer<CVector<4> > g_RegVarVector4;

const CRTTI *GetVectorVarRTTI(int iDim, bool bVarRef)
{
  static CRTTI const *pVectorRTTI[2][4] =
    { { 0, CVar<CVector<2> >::GetRTTI_s(),    CVar<CVector<3> >::GetRTTI_s(),    CVar<CVector<4> >::GetRTTI_s()    },

      { 0, CVarRef<CVector<2> >::GetRTTI_s(), CVarRef<CVector<3> >::GetRTTI_s(), CVarRef<CVector<4> >::GetRTTI_s() } };

  if (iDim < 1 || iDim > ARRSIZE(pVectorRTTI[0]))
    return 0;
  return pVectorRTTI[!!bVarRef][iDim - 1];
}


struct TVectorTest {
  TVectorTest() {
    CVector<3> v1 = {{0, 1, 2}}, v2, v3, v4, v5;

    v2.Set(3, 4, 5);

    v3.SetUnit(1);
    v3.x() = 5;
    v3.z() = 18;

    v3 = 2.0f * v1 + v2 * 2 + CVector<>::Get(6, 6, 6);

    v3.Normalize();

    v1.x() = v3.LengthSqr();
    v3 = v1.Perpendicular();
    v2.x() = v1 % v3;
    v2 = v1 ^ v3;
    v4 = v3 ^ v1;
    v5 = v2 + v4;
  }
} /*g_VectorTest*/;

